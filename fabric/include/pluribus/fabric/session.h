#pragma once

#include <cstdint>
#include <optional>
#include <utility>

#include "pluribus/fabric/frame_stream.h"
#include "pluribus/fabric/handshake.h"

namespace pluribus::fabric {

// Established control session. Assigns and validates the directional frame
// sequence after a successful Hello/Welcome handshake. Replay/resumption is
// deliberately not implemented in this slice: gaps and duplicates are errors.
class Session {
public:
    static Result<Session> accept(FrameStream stream, const HandshakePolicy& policy);
    static Result<Session> connect(FrameStream stream, const Hello& hello);

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
    Session(Session&&) noexcept = default;
    Session& operator=(Session&&) noexcept = default;

    Result<void> send(const Frame& frame);
    Result<std::optional<Frame>> receive();

    Result<void> close();

    const SessionId& id() const noexcept { return id_; }
    ProtocolVersion version() const noexcept { return version_; }
    const SessionLimits& limits() const noexcept { return limits_; }

private:
    Session(FrameStream stream, ProtocolVersion version, SessionId id, SessionLimits limits)
        : stream_(std::move(stream)),
          version_(version),
          id_(id),
          limits_(limits) {}

    FrameStream stream_;
    ProtocolVersion version_;
    SessionId id_;
    SessionLimits limits_;
    std::uint64_t next_send_sequence_ = 1;
    std::uint64_t next_receive_sequence_ = 1;
};

} // namespace pluribus::fabric
