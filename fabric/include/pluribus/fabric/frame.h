#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "pluribus/core/error.h"
#include "pluribus/core/version.h"

namespace pluribus::fabric {

inline constexpr std::uint8_t kBootstrapVersion = 0;
inline constexpr std::uint8_t kProtocolVersion = 1;

inline constexpr std::size_t kLengthPrefixBytes = kControlFrameLengthPrefixBytes;
inline constexpr std::size_t kHeaderBytes = kControlFrameHeaderBytes;
inline constexpr std::uint32_t kMinFrameLength =
    static_cast<std::uint32_t>(kHeaderBytes);
inline constexpr std::uint32_t kMaxFrameLength =
    static_cast<std::uint32_t>(kMaxControlPayloadBytes);

// All integers use network byte order on the wire. The length field is not
// part of this structure; it is derived from the payload size.
struct FrameHeader {
    std::uint8_t version = kBootstrapVersion;
    std::uint8_t type = 0;
    std::uint16_t flags = 0;
    std::uint64_t sequence = 0;

    friend bool operator==(const FrameHeader&, const FrameHeader&) = default;
};

struct Frame {
    FrameHeader header;
    std::vector<std::byte> payload;

    friend bool operator==(const Frame&, const Frame&) = default;
};

// Validation policy for a connection. Version zero remains acceptable for
// bootstrap messages even after negotiation; the session layer enforces
// which message types are legal in which phase.
struct FramingLimits {
    std::uint8_t max_supported_version = kProtocolVersion;
    std::uint16_t known_flags = 0;
};

Result<void> validate_header(const FrameHeader& header, const FramingLimits& limits = {});

Result<std::vector<std::byte>> encode_frame(
    const Frame& frame,
    const FramingLimits& limits = {});

// Decodes exactly one complete frame and rejects truncated or trailing bytes.
Result<Frame> decode_frame(
    std::span<const std::byte> frame_bytes,
    const FramingLimits& limits = {});

// Incremental decoder for a TCP byte stream. It never blocks and never
// assumes frame boundaries align with read boundaries. Structural or policy
// errors latch: after a framing error the connection must be closed.
class FrameDecoder {
public:
    explicit FrameDecoder(FramingLimits limits = {}) : limits_(limits) {}

    Result<void> push(std::span<const std::byte> bytes);

    // Returns nullopt when more bytes are needed. Errors are terminal for
    // this decoder.
    Result<std::optional<Frame>> next();

    std::size_t buffered_bytes() const noexcept { return buffer_.size() - offset_; }
    bool failed() const noexcept { return failure_.has_value(); }

private:
    void compact();

    FramingLimits limits_;
    std::vector<std::byte> buffer_;
    std::size_t offset_ = 0;
    std::optional<Error> failure_;
};

} // namespace pluribus::fabric
