#pragma once

#include <cstdint>

namespace pluribus::fabric {

// Provisional numeric assignments for the bootstrap handshake. The canonical
// numeric registry lives in fabric/proto/control.proto (Q-48); the values here
// must match MessageType::hello/welcome/reject and are enforced by
// static_asserts in fabric/src/control_codec.cpp. They are not a frozen
// contract.
inline constexpr std::uint8_t kBootstrapHelloType = 1;
inline constexpr std::uint8_t kBootstrapWelcomeType = 2;
inline constexpr std::uint8_t kBootstrapRejectType = 3;

constexpr bool is_bootstrap_message_type(std::uint8_t type) noexcept {
    return type == kBootstrapHelloType || type == kBootstrapWelcomeType ||
           type == kBootstrapRejectType;
}

} // namespace pluribus::fabric
