#include "pluribus/fabric/frame.h"

#include <string>

namespace pluribus::fabric {
namespace {

constexpr std::size_t kNetworkPrefixBytes = kLengthPrefixBytes;
constexpr std::size_t kHeaderVersionOffset = 0;
constexpr std::size_t kHeaderTypeOffset = 1;
constexpr std::size_t kHeaderFlagsOffset = 2;
constexpr std::size_t kHeaderSequenceOffset = 4;

std::uint16_t read_u16(const std::byte* bytes) noexcept {
    return static_cast<std::uint16_t>(
        (std::to_integer<std::uint16_t>(bytes[0]) << 8) |
        std::to_integer<std::uint16_t>(bytes[1]));
}

std::uint32_t read_u32(const std::byte* bytes) noexcept {
    return (std::to_integer<std::uint32_t>(bytes[0]) << 24) |
           (std::to_integer<std::uint32_t>(bytes[1]) << 16) |
           (std::to_integer<std::uint32_t>(bytes[2]) << 8) |
           std::to_integer<std::uint32_t>(bytes[3]);
}

std::uint64_t read_u64(const std::byte* bytes) noexcept {
    std::uint64_t value = 0;
    for (std::size_t index = 0; index < 8; ++index) {
        value = (value << 8) | std::to_integer<std::uint64_t>(bytes[index]);
    }
    return value;
}

void write_u16(std::byte* bytes, std::uint16_t value) noexcept {
    bytes[0] = static_cast<std::byte>((value >> 8) & 0xFF);
    bytes[1] = static_cast<std::byte>(value & 0xFF);
}

void write_u32(std::byte* bytes, std::uint32_t value) noexcept {
    bytes[0] = static_cast<std::byte>((value >> 24) & 0xFF);
    bytes[1] = static_cast<std::byte>((value >> 16) & 0xFF);
    bytes[2] = static_cast<std::byte>((value >> 8) & 0xFF);
    bytes[3] = static_cast<std::byte>(value & 0xFF);
}

void write_u64(std::byte* bytes, std::uint64_t value) noexcept {
    for (std::size_t index = 0; index < 8; ++index) {
        bytes[index] = static_cast<std::byte>((value >> ((7 - index) * 8)) & 0xFF);
    }
}

Result<std::uint32_t> read_frame_length(
    std::span<const std::byte> bytes,
    std::size_t offset) {
    if (bytes.size() - offset < kNetworkPrefixBytes) {
        return std::unexpected(
            make_error(ErrorCode::invalid_argument, "frame is shorter than its length prefix"));
    }

    const auto length = read_u32(bytes.data() + offset);
    if (length < kMinFrameLength) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument,
            "frame length " + std::to_string(length) + " is below the minimum " +
                std::to_string(kMinFrameLength)));
    }
    if (length > kMaxFrameLength) {
        return std::unexpected(make_error(
            ErrorCode::message_too_large,
            "frame length " + std::to_string(length) + " exceeds the maximum " +
                std::to_string(kMaxFrameLength)));
    }
    return length;
}

Result<Frame> parse_complete_frame(
    std::span<const std::byte> bytes,
    const FramingLimits& limits) {
    if (bytes.size() < kNetworkPrefixBytes) {
        return std::unexpected(
            make_error(ErrorCode::invalid_argument, "frame is shorter than its length prefix"));
    }

    const auto length_result = read_frame_length(bytes, 0);
    if (!length_result) {
        return std::unexpected(length_result.error());
    }
    const std::uint32_t length = *length_result;
    const std::size_t expected_size = kNetworkPrefixBytes + length;
    if (bytes.size() != expected_size) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument,
            "frame length prefix does not match the supplied byte count"));
    }

    const std::byte* header = bytes.data() + kNetworkPrefixBytes;
    Frame frame;
    frame.header.version = std::to_integer<std::uint8_t>(header[kHeaderVersionOffset]);
    frame.header.type = std::to_integer<std::uint8_t>(header[kHeaderTypeOffset]);
    frame.header.flags = read_u16(header + kHeaderFlagsOffset);
    frame.header.sequence = read_u64(header + kHeaderSequenceOffset);

    if (auto valid = validate_header(frame.header, limits); !valid) {
        return std::unexpected(valid.error());
    }

    const std::size_t payload_size = length - kHeaderBytes;
    const std::byte* payload_start = header + kHeaderBytes;
    frame.payload.assign(payload_start, payload_start + payload_size);
    return frame;
}

} // namespace

Result<void> validate_header(const FrameHeader& header, const FramingLimits& limits) {
    if (header.version > limits.max_supported_version) {
        return std::unexpected(make_error(
            ErrorCode::unsupported_version,
            "unsupported wire version " + std::to_string(header.version)));
    }
    if ((header.flags & static_cast<std::uint16_t>(~limits.known_flags)) != 0) {
        return std::unexpected(make_error(
            ErrorCode::unsupported_feature,
            "frame uses unknown mandatory flags"));
    }
    return {};
}

Result<std::vector<std::byte>> encode_frame(const Frame& frame, const FramingLimits& limits) {
    if (auto valid = validate_header(frame.header, limits); !valid) {
        return std::unexpected(valid.error());
    }
    if (frame.payload.size() > kMaxFrameLength - kHeaderBytes) {
        return std::unexpected(make_error(
            ErrorCode::message_too_large,
            "frame payload exceeds the control-message limit"));
    }

    const auto payload_size = static_cast<std::uint32_t>(frame.payload.size());
    const std::uint32_t length = static_cast<std::uint32_t>(kHeaderBytes) + payload_size;
    std::vector<std::byte> encoded(kNetworkPrefixBytes + length);

    write_u32(encoded.data(), length);
    std::byte* header = encoded.data() + kNetworkPrefixBytes;
    header[kHeaderVersionOffset] = static_cast<std::byte>(frame.header.version);
    header[kHeaderTypeOffset] = static_cast<std::byte>(frame.header.type);
    write_u16(header + kHeaderFlagsOffset, frame.header.flags);
    write_u64(header + kHeaderSequenceOffset, frame.header.sequence);

    std::byte* payload = header + kHeaderBytes;
    for (std::size_t index = 0; index < frame.payload.size(); ++index) {
        payload[index] = frame.payload[index];
    }
    return encoded;
}

Result<Frame> decode_frame(std::span<const std::byte> frame_bytes, const FramingLimits& limits) {
    return parse_complete_frame(frame_bytes, limits);
}

Result<void> FrameDecoder::push(std::span<const std::byte> bytes) {
    if (failure_) {
        return std::unexpected(*failure_);
    }
    buffer_.insert(buffer_.end(), bytes.begin(), bytes.end());
    return {};
}

Result<std::optional<Frame>> FrameDecoder::next() {
    if (failure_) {
        return std::unexpected(*failure_);
    }
    if (buffered_bytes() < kNetworkPrefixBytes) {
        return std::optional<Frame>{};
    }

    const auto length_result =
        read_frame_length(std::span<const std::byte>(buffer_).subspan(offset_), 0);
    if (!length_result) {
        failure_ = length_result.error();
        return std::unexpected(*failure_);
    }

    const std::uint32_t length = *length_result;
    const std::size_t total_size = kNetworkPrefixBytes + length;
    if (buffered_bytes() < total_size) {
        return std::optional<Frame>{};
    }

    const auto frame_bytes =
        std::span<const std::byte>(buffer_).subspan(offset_, total_size);
    auto frame = parse_complete_frame(frame_bytes, limits_);
    if (!frame) {
        failure_ = frame.error();
        return std::unexpected(*failure_);
    }

    offset_ += total_size;
    compact();
    return std::optional<Frame>(std::move(*frame));
}

void FrameDecoder::compact() {
    if (offset_ == buffer_.size()) {
        buffer_.clear();
        offset_ = 0;
        return;
    }
    if (offset_ >= 4096 && offset_ * 2 >= buffer_.size()) {
        buffer_.erase(
            buffer_.begin(),
            buffer_.begin() + static_cast<std::ptrdiff_t>(offset_));
        offset_ = 0;
    }
}

} // namespace pluribus::fabric
