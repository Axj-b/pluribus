#include "pluribus/core/node.h"

namespace pluribus {

std::string_view to_string(NodeHealth health) {
    switch (health) {
    case NodeHealth::healthy:
        return "healthy";
    case NodeHealth::degraded:
        return "degraded";
    case NodeHealth::unhealthy:
        return "unhealthy";
    }
    return "unhealthy";
}

std::optional<NodeHealth> parse_node_health(std::string_view text) {
    if (text == "healthy") {
        return NodeHealth::healthy;
    }
    if (text == "degraded") {
        return NodeHealth::degraded;
    }
    if (text == "unhealthy") {
        return NodeHealth::unhealthy;
    }
    return std::nullopt;
}

} // namespace pluribus
