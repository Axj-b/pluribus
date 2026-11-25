#pragma once

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <string_view>

#include "pluribus/core/ids.h"
#include "pluribus/core/names.h"
#include "pluribus/core/resources.h"

namespace pluribus {

enum class NodeHealth : std::uint8_t {
    healthy,
    degraded,
    unhealthy,
};

std::string_view to_string(NodeHealth health);
std::optional<NodeHealth> parse_node_health(std::string_view text);

struct NodeDescriptor {
    NodeId id;
    std::string name;
    std::uint32_t protocol_major = 0;
    AbiVersion abi_version;
    CpuArchitecture architecture = CpuArchitecture::x86_64;
    std::uint32_t logical_cpu_count = 0;
    MemoryBytes total_memory_bytes = 0;
    std::set<CapabilityName, std::less<>> capabilities;
    std::set<CapabilityName, std::less<>> devices;
};

struct NodeStatus {
    Revision descriptor_revision = 0;
    NodeHealth health = NodeHealth::unhealthy;
    bool online = false;
    bool draining = false;
    std::uint32_t running_executions = 0;
};

// Optional measurements: absent means unknown, never zero.
struct NodeMetrics {
    std::optional<LoadBasisPoints> cpu_load;
    std::optional<Timestamp> cpu_observed_at;
    std::optional<MemoryBytes> available_memory_bytes;
    std::optional<Timestamp> memory_observed_at;
};

struct NodeSnapshot {
    NodeDescriptor descriptor;
    NodeStatus status;
    NodeMetrics metrics;
};

} // namespace pluribus
