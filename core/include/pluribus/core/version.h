#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace pluribus {

inline constexpr std::uint16_t kWireProtocolMajor = 1;
inline constexpr std::uint16_t kWireProtocolMinor = 0;

inline constexpr std::string_view kPluribusAbi = "pluribus-v1";

// Fixed control framing from docs/spec/05-wire-protocol.md.
inline constexpr std::size_t kControlFrameHeaderBytes = 12;
inline constexpr std::size_t kControlFrameLengthPrefixBytes = 4;
inline constexpr std::size_t kMaxControlFrameBytes = 1'048'576;
inline constexpr std::size_t kMaxControlPayloadBytes =
    kMaxControlFrameBytes - kControlFrameLengthPrefixBytes;

} // namespace pluribus
