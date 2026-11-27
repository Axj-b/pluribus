#include "pluribus/fabric/frame_stream.h"

#include <array>
#include <span>
#include <utility>

namespace pluribus::fabric {
namespace {

constexpr std::size_t kReadBufferBytes = 64 * 1024;

} // namespace

FrameStream::FrameStream(std::unique_ptr<Stream> stream, FramingLimits limits)
    : stream_(std::move(stream)), decoder_(limits), limits_(limits) {}

Result<void> FrameStream::write_frame(const Frame& frame) {
    auto encoded = encode_frame(frame, limits_);
    if (!encoded) {
        return std::unexpected(encoded.error());
    }
    return stream_->write(*encoded);
}

Result<std::optional<Frame>> FrameStream::read_frame() {
    std::array<std::byte, kReadBufferBytes> buffer{};

    for (;;) {
        auto decoded = decoder_.next();
        if (!decoded) {
            return std::unexpected(decoded.error());
        }
        if (decoded->has_value()) {
            return decoded;
        }

        auto read = stream_->read(buffer);
        if (!read) {
            return std::unexpected(read.error());
        }
        if (*read == 0) {
            if (decoder_.buffered_bytes() != 0) {
                return std::unexpected(make_error(
                    ErrorCode::invalid_argument,
                    "stream ended in the middle of a frame"));
            }
            return std::optional<Frame>{};
        }

        auto pushed = decoder_.push(std::span<const std::byte>(buffer).first(*read));
        if (!pushed) {
            return std::unexpected(pushed.error());
        }
    }
}

Result<void> FrameStream::close() {
    return stream_->close();
}

} // namespace pluribus::fabric
