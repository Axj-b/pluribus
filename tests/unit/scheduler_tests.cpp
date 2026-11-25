#include "test.h"

#include <pluribus/scheduler/scheduler.h>

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

using namespace pluribus;
using namespace pluribus::scheduler;

constexpr MemoryBytes GiB = 1ULL << 30;

AbiVersion abi() {
    return *AbiVersion::parse("pluribus-v1");
}

NodeId make_node_id(std::uint8_t last_byte) {
    static constexpr char hex[] = "0123456789abcdef";
    std::string text = "00000000-0000-0000-0000-0000000000";
    text.push_back(hex[last_byte >> 4]);
    text.push_back(hex[last_byte & 0x0F]);
    return *NodeId::parse(text);
}

CapabilityName capability(std::string_view text) {
    return *CapabilityName::parse(text);
}

Timestamp snapshot_time() {
    return Timestamp{} + std::chrono::seconds(100);
}

SchedulerNode make_node(
    std::uint8_t id,
    std::uint64_t cpu_millicores = 8'000,
    std::uint64_t memory_bytes = 16 * GiB) {
    SchedulerNode node;
    node.id = make_node_id(id);
    node.abi_version = abi();
    node.architecture = CpuArchitecture::x86_64;
    node.total_cpu_millicores = cpu_millicores;
    node.total_memory_bytes = memory_bytes;
    node.online = true;
    node.health = NodeHealth::healthy;
    node.measured_cpu_load_basis_points = 0;
    node.available_memory_bytes = memory_bytes;
    node.cpu_observed_at = snapshot_time() - std::chrono::seconds(1);
    node.memory_observed_at = snapshot_time() - std::chrono::seconds(1);
    return node;
}

SchedulerPolicy make_policy() {
    SchedulerPolicy policy;
    policy.default_cpu_millicores = 1'000;
    policy.default_memory_bytes = GiB;
    policy.snapshot_time = snapshot_time();
    policy.max_measurement_age = std::chrono::seconds(5);
    return policy;
}

ScheduleRequest make_request() {
    ScheduleRequest request;
    request.compatible_keys.insert(CompatibilityKey{abi(), CpuArchitecture::x86_64});
    return request;
}

std::optional<Selection> selection_of(const ScheduleDecision& decision) {
    if (!decision) {
        return std::nullopt;
    }
    if (const auto* selection = std::get_if<Selection>(&*decision)) {
        return *selection;
    }
    return std::nullopt;
}

bool is_fundamental(const ScheduleDecision& decision) {
    return decision && std::holds_alternative<FundamentallyIncompatible>(*decision);
}

bool is_temporary(const ScheduleDecision& decision) {
    return decision && std::holds_alternative<TemporarilyUnschedulable>(*decision);
}

} // namespace

PLURIBUS_TEST(scheduler_worked_example_prefers_lower_projected_load) {
    auto node_a = make_node(1);
    node_a.measured_cpu_load_basis_points = 2'500;
    node_a.available_memory_bytes = 12 * GiB;

    auto node_b = make_node(2);
    node_b.measured_cpu_load_basis_points = 1'250;
    node_b.available_memory_bytes = 14 * GiB;

    auto policy = make_policy();
    policy.local_node = node_a.id;

    const std::vector<SchedulerNode> nodes{node_a, node_b};
    const auto decision = schedule(make_request(), nodes, policy);
    const auto selection = selection_of(decision);
    CHECK(selection.has_value());
    CHECK(selection->node == node_b.id);
    CHECK(selection->reserve_cpu_millicores == 1'000);
    CHECK(selection->reserve_memory_bytes == GiB);
}

PLURIBUS_TEST(scheduler_exact_tie_prefers_local_node) {
    auto node_a = make_node(1);
    node_a.measured_cpu_load_basis_points = 2'500;
    node_a.available_memory_bytes = 12 * GiB;

    auto node_b = make_node(2);
    node_b.measured_cpu_load_basis_points = 2'500;
    node_b.available_memory_bytes = 12 * GiB;

    auto policy = make_policy();
    policy.local_node = node_a.id;

    const std::vector<SchedulerNode> nodes{node_a, node_b};
    const auto selection = selection_of(schedule(make_request(), nodes, policy));
    CHECK(selection.has_value());
    CHECK(selection->node == node_a.id);
}

PLURIBUS_TEST(scheduler_exact_tie_uses_lexicographic_node_uuid) {
    auto node_a = make_node(1);
    auto node_b = make_node(2);

    const std::vector<SchedulerNode> nodes{node_b, node_a};
    const auto selection = selection_of(schedule(make_request(), nodes, make_policy()));
    CHECK(selection.has_value());
    CHECK(selection->node == node_a.id);
}

PLURIBUS_TEST(scheduler_node_order_does_not_change_result) {
    auto node_a = make_node(1);
    node_a.measured_cpu_load_basis_points = 4'000;
    node_a.available_memory_bytes = 10 * GiB;

    auto node_b = make_node(2);
    node_b.measured_cpu_load_basis_points = 1'000;
    node_b.available_memory_bytes = 14 * GiB;

    const auto policy = make_policy();
    const std::vector<SchedulerNode> forward{node_a, node_b};
    const std::vector<SchedulerNode> reverse{node_b, node_a};

    const auto first = selection_of(schedule(make_request(), forward, policy));
    const auto second = selection_of(schedule(make_request(), reverse, policy));
    CHECK(first.has_value());
    CHECK(second.has_value());
    CHECK(first->node == node_b.id);
    CHECK(second->node == node_b.id);
}

PLURIBUS_TEST(scheduler_rejects_incompatible_architecture_and_abi) {
    auto node = make_node(1);
    auto request = make_request();
    request.compatible_keys = {CompatibilityKey{abi(), CpuArchitecture::arm64}};
    CHECK(is_fundamental(schedule(request, std::vector<SchedulerNode>{node}, make_policy())));

    request = make_request();
    request.compatible_keys = {
        CompatibilityKey{*AbiVersion::parse("pluribus-v2"), CpuArchitecture::x86_64}};
    CHECK(is_fundamental(schedule(request, std::vector<SchedulerNode>{node}, make_policy())));
}

PLURIBUS_TEST(scheduler_filters_required_capabilities_and_devices) {
    auto node_a = make_node(1);
    node_a.capabilities.insert(capability("gpu.cuda"));
    node_a.devices.insert(capability("device.radar.front"));

    auto node_b = make_node(2);
    node_b.capabilities.insert(capability("gpu.vulkan"));

    auto request = make_request();
    request.required_capabilities.insert(capability("gpu.cuda"));
    auto selection = selection_of(
        schedule(request, std::vector<SchedulerNode>{node_a, node_b}, make_policy()));
    CHECK(selection.has_value());
    CHECK(selection->node == node_a.id);

    request = make_request();
    request.required_capabilities.insert(capability("target.thor"));
    CHECK(is_fundamental(
        schedule(request, std::vector<SchedulerNode>{node_a, node_b}, make_policy())));

    request = make_request();
    request.required_devices.insert(capability("device.radar.front"));
    selection = selection_of(
        schedule(request, std::vector<SchedulerNode>{node_a, node_b}, make_policy()));
    CHECK(selection.has_value());
    CHECK(selection->node == node_a.id);
}

PLURIBUS_TEST(scheduler_honors_explicit_node) {
    auto node_a = make_node(1);
    node_a.measured_cpu_load_basis_points = 500;
    auto node_b = make_node(2);
    node_b.measured_cpu_load_basis_points = 4'000;

    auto request = make_request();
    request.explicit_node = node_b.id;
    auto selection = selection_of(
        schedule(request, std::vector<SchedulerNode>{node_a, node_b}, make_policy()));
    CHECK(selection.has_value());
    CHECK(selection->node == node_b.id);

    request.explicit_node = make_node_id(9);
    CHECK(is_fundamental(
        schedule(request, std::vector<SchedulerNode>{node_a, node_b}, make_policy())));
}

PLURIBUS_TEST(scheduler_queues_when_no_live_node_is_available) {
    auto offline = make_node(1);
    offline.online = false;
    CHECK(is_temporary(schedule(make_request(), std::vector<SchedulerNode>{offline}, make_policy())));

    auto degraded = make_node(1);
    degraded.health = NodeHealth::degraded;
    CHECK(is_temporary(
        schedule(make_request(), std::vector<SchedulerNode>{degraded}, make_policy())));

    auto draining = make_node(1);
    draining.draining = true;
    CHECK(is_temporary(
        schedule(make_request(), std::vector<SchedulerNode>{draining}, make_policy())));
}

PLURIBUS_TEST(scheduler_queues_without_fresh_measurements) {
    auto missing_cpu = make_node(1);
    missing_cpu.cpu_observed_at.reset();
    CHECK(is_temporary(
        schedule(make_request(), std::vector<SchedulerNode>{missing_cpu}, make_policy())));

    auto stale = make_node(1);
    stale.cpu_observed_at = snapshot_time() - std::chrono::seconds(10);
    CHECK(is_temporary(schedule(make_request(), std::vector<SchedulerNode>{stale}, make_policy())));

    auto future = make_node(1);
    future.memory_observed_at = snapshot_time() + std::chrono::seconds(1);
    CHECK(is_temporary(schedule(make_request(), std::vector<SchedulerNode>{future}, make_policy())));
}

PLURIBUS_TEST(scheduler_uses_requested_or_default_reservations) {
    auto node = make_node(1);
    auto request = make_request();
    auto selection =
        selection_of(schedule(request, std::vector<SchedulerNode>{node}, make_policy()));
    CHECK(selection.has_value());
    CHECK(selection->reserve_cpu_millicores == 1'000);
    CHECK(selection->reserve_memory_bytes == GiB);

    request.cpu = 500;
    request.memory = 2 * GiB;
    selection = selection_of(schedule(request, std::vector<SchedulerNode>{node}, make_policy()));
    CHECK(selection.has_value());
    CHECK(selection->reserve_cpu_millicores == 500);
    CHECK(selection->reserve_memory_bytes == 2 * GiB);
}

PLURIBUS_TEST(scheduler_rejects_invalid_request_and_policy) {
    auto node = make_node(1);

    auto request = make_request();
    request.compatible_keys.clear();
    auto decision = schedule(request, std::vector<SchedulerNode>{node}, make_policy());
    CHECK(!decision.has_value());
    CHECK(decision.error().code == ErrorCode::invalid_argument);

    request = make_request();
    request.cpu = -1;
    decision = schedule(request, std::vector<SchedulerNode>{node}, make_policy());
    CHECK(!decision.has_value());
    CHECK(decision.error().code == ErrorCode::invalid_argument);

    request = make_request();
    request.required_capabilities.insert(CapabilityName{});
    decision = schedule(request, std::vector<SchedulerNode>{node}, make_policy());
    CHECK(!decision.has_value());
    CHECK(decision.error().code == ErrorCode::invalid_argument);

    request = make_request();
    request.input_artifacts.insert(ArtifactId{});
    decision = schedule(request, std::vector<SchedulerNode>{node}, make_policy());
    CHECK(!decision.has_value());
    CHECK(decision.error().code == ErrorCode::invalid_argument);

    auto policy = make_policy();
    policy.default_memory_bytes = 0;
    decision = schedule(make_request(), std::vector<SchedulerNode>{node}, policy);
    CHECK(!decision.has_value());
    CHECK(decision.error().code == ErrorCode::invalid_argument);
}

PLURIBUS_TEST(scheduler_admits_exact_capacity_and_rejects_smaller_totals) {
    auto exact = make_node(1, 1'000, GiB);
    const auto selection =
        selection_of(schedule(make_request(), std::vector<SchedulerNode>{exact}, make_policy()));
    CHECK(selection.has_value());
    CHECK(selection->node == exact.id);

    auto too_small = make_node(1, 999, GiB);
    CHECK(is_fundamental(
        schedule(make_request(), std::vector<SchedulerNode>{too_small}, make_policy())));
}

PLURIBUS_TEST(scheduler_accounts_for_existing_reservations) {
    auto node = make_node(1, 2'000, 4 * GiB);
    node.available_memory_bytes = 2 * GiB;
    node.reserved_cpu_millicores = 500;
    node.reserved_memory_bytes = 128ULL << 20;

    auto request = make_request();
    request.cpu = 1'500;
    auto selection =
        selection_of(schedule(request, std::vector<SchedulerNode>{node}, make_policy()));
    CHECK(selection.has_value());

    request.cpu = 1'600;
    CHECK(is_temporary(schedule(request, std::vector<SchedulerNode>{node}, make_policy())));

    request = make_request();
    request.memory = 2 * GiB;
    CHECK(is_temporary(schedule(request, std::vector<SchedulerNode>{node}, make_policy())));
}

PLURIBUS_TEST(scheduler_clamps_out_of_range_measurements) {
    auto node = make_node(1, 2'000, 2 * GiB);
    node.measured_cpu_load_basis_points = 20'000;
    node.available_memory_bytes = 4 * GiB;

    auto request = make_request();
    request.cpu = 0;
    request.memory = 0;
    const auto selection =
        selection_of(schedule(request, std::vector<SchedulerNode>{node}, make_policy()));
    CHECK(selection.has_value());
}

PLURIBUS_TEST(scheduler_distinguishes_fundamental_incompatibility) {
    auto request = make_request();
    request.cpu = 3'000;
    auto node = make_node(1, 2'000, 16 * GiB);
    CHECK(is_fundamental(
        schedule(request, std::vector<SchedulerNode>{node}, make_policy())));

    CHECK(is_fundamental(
        schedule(make_request(), std::vector<SchedulerNode>{}, make_policy())));
}

PLURIBUS_TEST(scheduler_ignores_cache_history_and_labels) {
    auto node_a = make_node(1);
    node_a.cached_artifacts.insert(*ArtifactId::parse("sha256:" + std::string(64, 'a')));
    auto node_b = make_node(2);

    HistoricalProfile history;
    const std::vector<SchedulerNode> nodes{node_b, node_a};
    const auto selection =
        selection_of(schedule(make_request(), nodes, make_policy(), &history));
    CHECK(selection.has_value());
    CHECK(selection->node == node_a.id);
}
