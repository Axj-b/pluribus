#pragma once

#include <memory>
#include <optional>

#include "pluribus/fabric/frame.h"
#include "pluribus/fabric/stream.h"

namespace pluribus::fabric {

// Frames control messages over a blocking byte stream. Not thread-safe: one
// reader and one writer at a time, with external sequencing/session policy.
class FrameStream {
public:
    explicit FrameStream(std::unique_ptr<Stream> stream, FramingLimits limits = {});

    FrameStream(const FrameStream&) = delete;
    FrameStream& operator=(const FrameStream&) = delete;
    FrameStream(FrameStream&&) noexcept = default;
    FrameStream& operator=(FrameStream&&) noexcept = default;

    Result<void> write_frame(const Frame& frame);
    Result<std::optional<Frame>> read_frame();

    Result<void> close();
    Stream& stream() noexcept { return *stream_; }

private:
    std::unique_ptr<Stream> stream_;
    FrameDecoder decoder_;
    FramingLimits limits_;
};

} // namespace pluribus::fabric
