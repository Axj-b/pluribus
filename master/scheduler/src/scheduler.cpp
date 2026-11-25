#include "pluribus/scheduler/scheduler.h"

#include "wide_uint.h"

#include <algorithm>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace pluribus::scheduler {
namespace {

constexpr std::uint64_t kFullLoad = kFullLoadBasisPoints;

bool contains_all(
    const std::set<CapabilityName, std::less<>>& available,
    const std::set<CapabilityName, std::less<>>& required) {
    for (const auto& name : required) {
        if (available.find(name) == available.end()) {
            return false;
        }
    }
    return true;
}

bool matches_compatibility_key(
    const SchedulerNode& node,
    const std::set<CompatibilityKey>& keys) {
    if (node.abi_version.empty()) {
        return false;
    }
    for (const auto& key : keys) {
        if (node.abi_version == key.abi && node.architecture == key.architecture) {
            return true;
        }
    }
    return false;
}

bool is_fresh(
    std::optional<Timestamp> observed,
    Timestamp snapshot_time,
    std::chrono::milliseconds max_age) {
    if (!observed) {
        return false;
    }
    if (*observed > snapshot_time) {
        return false;
    }
    return snapshot_time - *observed <= max_age;
}

std::optional<Error> validate_request(
    const ScheduleRequest& request,
    const SchedulerPolicy& policy) {
    if (request.compatible_keys.empty()) {
        return make_error(
            ErrorCode::invalid_argument,
            "schedule request must contain at least one compatibility key");
    }
    for (const auto& key : request.compatible_keys) {
        if (key.abi.empty()) {
            return make_error(
                ErrorCode::invalid_argument,
                "schedule request contains an empty ABI version");
        }
    }
    if (request.cpu && *request.cpu < 0) {
        return make_error(ErrorCode::invalid_argument, "requested CPU must not be negative");
    }
    for (const auto& name : request.required_capabilities) {
        if (name.empty()) {
            return make_error(
                ErrorCode::invalid_argument,
                "required capability name must not be empty");
        }
    }
    for (const auto& name : request.required_devices) {
        if (name.empty()) {
            return make_error(
                ErrorCode::invalid_argument,
                "required device name must not be empty");
        }
    }
    for (const auto& artifact : request.input_artifacts) {
        if (artifact.is_nil()) {
            return make_error(
                ErrorCode::invalid_argument,
                "input artifact ID must not be nil");
        }
    }
    if (request.explicit_node && request.explicit_node->is_nil()) {
        return make_error(ErrorCode::invalid_argument, "explicit node must not be nil");
    }
    if (policy.default_cpu_millicores <= 0) {
        return make_error(
            ErrorCode::invalid_argument,
            "default CPU reservation must be positive");
    }
    if (policy.default_memory_bytes == 0) {
        return make_error(
            ErrorCode::invalid_argument,
            "default memory reservation must be positive");
    }
    if (policy.max_measurement_age < std::chrono::milliseconds::zero()) {
        return make_error(
            ErrorCode::invalid_argument,
            "maximum measurement age must not be negative");
    }
    return std::nullopt;
}

struct Candidate {
    const SchedulerNode* node = nullptr;
    detail::UInt128 numerator{};
    detail::UInt128 denominator{};
};

std::optional<Candidate> evaluate_candidate(
    const SchedulerNode& node,
    std::uint64_t reserve_cpu,
    std::uint64_t reserve_memory) {
    const auto max = std::numeric_limits<std::uint64_t>::max();

    if (node.reserved_cpu_millicores > max - reserve_cpu) {
        return std::nullopt;
    }
    const std::uint64_t reserved_cpu_total = node.reserved_cpu_millicores + reserve_cpu;

    const std::uint64_t load = std::min<std::uint64_t>(
        node.measured_cpu_load_basis_points.value_or(0), kFullLoad);
    const std::uint64_t available_memory =
        std::min<std::uint64_t>(node.available_memory_bytes.value_or(0), node.total_memory_bytes);

    detail::UInt128 used_cpu{};
    if (!detail::add(
            detail::multiply_u64(load, node.total_cpu_millicores),
            detail::multiply_u64(reserved_cpu_total, 10'000),
            used_cpu)) {
        return std::nullopt;
    }
    detail::UInt128 cpu_capacity{};
    if (!detail::multiply(
            detail::from_u64(node.total_cpu_millicores), 10'000, cpu_capacity)) {
        return std::nullopt;
    }
    if (detail::compare(used_cpu, cpu_capacity) > 0) {
        return std::nullopt;
    }

    if (node.reserved_memory_bytes > max - reserve_memory) {
        return std::nullopt;
    }
    const std::uint64_t reserved_memory_total =
        node.reserved_memory_bytes + reserve_memory;
    const std::uint64_t used_memory = node.total_memory_bytes - available_memory;
    if (used_memory > max - reserved_memory_total) {
        return std::nullopt;
    }
    const std::uint64_t needed_memory = used_memory + reserved_memory_total;
    if (needed_memory > node.total_memory_bytes) {
        return std::nullopt;
    }

    // score = ((usedCpu + reserveCpu) / totalCpu +
    //          (usedMemory + reserveMemory) / totalMemory) / 2
    // Compare exactly as a rational. Projecting by
    // 2 * 10000 * totalCpu * totalMemory yields:
    //   numerator = (load*totalCpu + 10000*reservedCpuTotal) * totalMemory
    //             + 10000 * totalCpu * neededMemory
    //   denominator = 2 * 10000 * totalCpu * totalMemory
    detail::UInt128 cpu_projection{};
    if (!detail::add(
            detail::multiply_u64(load, node.total_cpu_millicores),
            detail::multiply_u64(reserved_cpu_total, 10'000),
            cpu_projection)) {
        return std::nullopt;
    }

    detail::UInt128 memory_term{};
    if (!detail::multiply(cpu_projection, node.total_memory_bytes, memory_term)) {
        return std::nullopt;
    }

    const detail::UInt128 cpu_weight =
        detail::multiply_u64(node.total_cpu_millicores, 10'000);
    detail::UInt128 cpu_term{};
    if (!detail::multiply(cpu_weight, needed_memory, cpu_term)) {
        return std::nullopt;
    }

    detail::UInt128 numerator{};
    if (!detail::add(memory_term, cpu_term, numerator)) {
        return std::nullopt;
    }

    const detail::UInt128 score_base =
        detail::multiply_u64(node.total_cpu_millicores, node.total_memory_bytes);
    detail::UInt128 denominator{};
    if (!detail::multiply(score_base, 20'000, denominator)) {
        return std::nullopt;
    }

    return Candidate{&node, numerator, denominator};
}

bool is_better(
    const Candidate& candidate,
    const Candidate& current,
    const std::optional<NodeId>& local_node) {
    const auto candidate_score =
        detail::multiply(candidate.numerator, current.denominator);
    const auto current_score =
        detail::multiply(current.numerator, candidate.denominator);
    const int comparison = detail::compare(candidate_score, current_score);
    if (comparison != 0) {
        return comparison < 0;
    }

    const bool candidate_is_local = local_node && candidate.node->id == *local_node;
    const bool current_is_local = local_node && current.node->id == *local_node;
    if (candidate_is_local != current_is_local) {
        return candidate_is_local;
    }

    return candidate.node->id < current.node->id;
}

} // namespace

ScheduleDecision schedule(
    const ScheduleRequest& request,
    std::span<const SchedulerNode> nodes,
    const SchedulerPolicy& policy,
    const HistoricalProfile* historical_profile) {
    (void)historical_profile;

    if (auto error = validate_request(request, policy)) {
        return std::unexpected(std::move(*error));
    }

    const std::uint64_t reserve_cpu = request.cpu
        ? static_cast<std::uint64_t>(*request.cpu)
        : static_cast<std::uint64_t>(policy.default_cpu_millicores);
    const std::uint64_t reserve_memory =
        request.memory.value_or(policy.default_memory_bytes);

    std::vector<const SchedulerNode*> static_matches;
    static_matches.reserve(nodes.size());
    for (const auto& node : nodes) {
        if (node.id.is_nil()) {
            continue;
        }
        if (node.total_cpu_millicores == 0 || node.total_memory_bytes == 0) {
            continue;
        }
        if (request.explicit_node && !(node.id == *request.explicit_node)) {
            continue;
        }
        if (!matches_compatibility_key(node, request.compatible_keys)) {
            continue;
        }
        if (!contains_all(node.capabilities, request.required_capabilities)) {
            continue;
        }
        if (!contains_all(node.devices, request.required_devices)) {
            continue;
        }
        if (reserve_cpu > node.total_cpu_millicores ||
            reserve_memory > node.total_memory_bytes) {
            continue;
        }
        static_matches.push_back(&node);
    }

    if (static_matches.empty()) {
        return ScheduleResult(FundamentallyIncompatible{
            "no registered node matches compatibility, capability, device and capacity "
            "requirements"});
    }

    std::vector<Candidate> candidates;
    candidates.reserve(static_matches.size());
    for (const auto* node : static_matches) {
        if (!node->online || node->health != NodeHealth::healthy || node->draining) {
            continue;
        }
        if (!is_fresh(node->cpu_observed_at, policy.snapshot_time, policy.max_measurement_age) ||
            !is_fresh(
                node->memory_observed_at, policy.snapshot_time, policy.max_measurement_age)) {
            continue;
        }
        if (auto candidate = evaluate_candidate(*node, reserve_cpu, reserve_memory)) {
            candidates.push_back(*candidate);
        }
    }

    if (candidates.empty()) {
        return ScheduleResult(TemporarilyUnschedulable{
            "no online, healthy node with fresh measurements can admit the reservation"});
    }

    const Candidate* best = &candidates.front();
    for (std::size_t index = 1; index < candidates.size(); ++index) {
        if (is_better(candidates[index], *best, policy.local_node)) {
            best = &candidates[index];
        }
    }

    return ScheduleResult(Selection{
        best->node->id,
        reserve_cpu,
        reserve_memory,
    });
}

} // namespace pluribus::scheduler
