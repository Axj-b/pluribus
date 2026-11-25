#pragma once

#include <cstdint>

namespace pluribus::fabric {

// Provisional numeric assignments for the bootstrap handshake. The wire
// specification explicitly leaves the complete type registry open; these
// values are reserved for interoperability work and are not a frozen contract.
inline constexpr std::uint8_t kBootstrapHelloType = 1;
inline constexpr std::uint8_t kBootstrapWelcomeType = 2;
inline constexpr std::uint8_t kBootstrapRejectType = 3;

constexpr bool is_bootstrap_message_type(std::uint8_t type) noexcept {
    return type == kBootstrapHelloType || type == kBootstrapWelcomeType ||
           type == kBootstrapRejectType;
}

} // namespace pluribus::fabric
