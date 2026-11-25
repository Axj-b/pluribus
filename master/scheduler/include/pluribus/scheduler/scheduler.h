#pragma once

#include <chrono>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <variant>

#include "pluribus/core/error.h"
#include "pluribus/core/ids.h"
#include "pluribus/core/names.h"
#include "pluribus/core/node.h"
#include "pluribus/core/resources.h"

namespace pluribus::scheduler {

struct ScheduleRequest {
    // Keys come from the resolved executable/package, intersected with any
    // explicit ABI/architecture constraints.
    std::set<CompatibilityKey> compatible_keys;
    std::set<CapabilityName, std::less<>> required_capabilities;
    std::set<CapabilityName, std::less<>> required_devices;
    std::optional<NodeId> explicit_node;
    std::optional<CpuMillicores> cpu;
    std::optional<MemoryBytes> memory;
    std::set<ArtifactId> input_artifacts;
};

struct SchedulerNode {
    NodeId id;
    AbiVersion abi_version;
    CpuArchitecture architecture = CpuArchitecture::x86_64;
    std::set<CapabilityName, std::less<>> capabilities;
    std::set<CapabilityName, std::less<>> devices;
    std::uint64_t total_cpu_millicores = 0;
    std::uint64_t total_memory_bytes = 0;
    bool online = false;
    NodeHealth health = NodeHealth::unhealthy;
    bool draining = false;
    std::optional<LoadBasisPoints> measured_cpu_load_basis_points;
    std::optional<Timestamp> cpu_observed_at;
    std::optional<MemoryBytes> available_memory_bytes;
    std::optional<Timestamp> memory_observed_at;
    std::uint64_t reserved_cpu_millicores = 0;
    std::uint64_t reserved_memory_bytes = 0;
    std::set<ArtifactId> cached_artifacts;
};

struct SchedulerPolicy {
    CpuMillicores default_cpu_millicores = 1000;
    MemoryBytes default_memory_bytes = 1ULL << 30;
    std::optional<NodeId> local_node;
    Timestamp snapshot_time{};
    std::chrono::milliseconds max_measurement_age = std::chrono::seconds(5);
};

struct Selection {
    NodeId node;
    std::uint64_t reserve_cpu_millicores = 0;
    std::uint64_t reserve_memory_bytes = 0;
};

struct TemporarilyUnschedulable {
    std::string reason;
};

struct FundamentallyIncompatible {
    std::string reason;
};

using ScheduleResult = std::variant<Selection, TemporarilyUnschedulable, FundamentallyIncompatible>;
using ScheduleDecision = std::expected<ScheduleResult, Error>;

// Explicit future hook. V1 ignores historical profiles entirely.
struct HistoricalProfile {};

// Pure scheduling function: queueing, reservation and dispatch belong to the
// master. The returned selection is a recommendation; the master must
// revalidate the snapshot before committing it.
ScheduleDecision schedule(
    const ScheduleRequest& request,
    std::span<const SchedulerNode> nodes,
    const SchedulerPolicy& policy,
    const HistoricalProfile* historical_profile = nullptr);

} // namespace pluribus::scheduler
