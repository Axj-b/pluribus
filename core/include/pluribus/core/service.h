#pragma once

#include <cstdint>
#include <string>

#include "pluribus/core/ids.h"
#include "pluribus/core/names.h"

namespace pluribus {

enum class ServiceMode : std::uint8_t {
    byte_stream,
};

enum class ServiceReadiness : std::uint8_t {
    created,
    ready,
};

// Registration of one provider instance. The endpoint is internal routing
// metadata and never a public host address.
struct ServiceDescriptor {
    ServiceId id;
    ServiceName name;
    ExecutionId execution;
    AssignmentId assignment;
    NodeId node;
    ServiceMode mode = ServiceMode::byte_stream;
    std::string protocol_tag;
    std::string endpoint;
    ServiceReadiness readiness = ServiceReadiness::created;
};

} // namespace pluribus
