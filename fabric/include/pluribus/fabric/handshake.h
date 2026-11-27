#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "pluribus/core/error.h"
#include "pluribus/core/ids.h"
#include "pluribus/fabric/frame.h"

namespace pluribus::fabric {

struct SessionIdTag;
using SessionId = UuidId<SessionIdTag>;

enum class SessionRole : std::uint8_t {
    node = 1,
    client = 2,
};

struct ProtocolVersion {
    std::uint8_t major = 1;
    std::uint8_t minor = 0;

    friend bool operator==(const ProtocolVersion&, const ProtocolVersion&) = default;
    friend auto operator<=>(const ProtocolVersion&, const ProtocolVersion&) = default;
};

struct SessionLimits {
    std::uint32_t max_control_payload =
        static_cast<std::uint32_t>(kMaxFrameLength - kHeaderBytes);
    std::uint32_t max_outstanding_requests = 16;
};

struct Hello {
    SessionRole role = SessionRole::client;
    std::vector<ProtocolVersion> versions;
    std::uint64_t features = 0;
    std::string identity;
    std::optional<std::string> join_token;
    std::optional<SessionId> prior_session;
};

struct Welcome {
    ProtocolVersion version;
    SessionId session;
    std::uint64_t features = 0;
    SessionLimits limits;
    bool resumed = false;
};

enum class HandshakeRejectCode : std::uint16_t {
    unsupported_version = 1,
    unsupported_feature = 2,
    unauthorized = 3,
    invalid_argument = 4,
    internal = 5,
};

struct Reject {
    HandshakeRejectCode code = HandshakeRejectCode::internal;
    std::string diagnostic;
};

ErrorCode to_error_code(HandshakeRejectCode code) noexcept;

// Provisional binary bootstrap codec used to prove the handshake and session
// layer. It is not the final serializer and is not part of the stable wire
// contract; replace it when the OI-14 codec decision is made.
Result<std::vector<std::byte>> encode_hello(const Hello& hello);
Result<Hello> decode_hello(std::span<const std::byte> payload);

Result<std::vector<std::byte>> encode_welcome(const Welcome& welcome);
Result<Welcome> decode_welcome(std::span<const std::byte> payload);

Result<std::vector<std::byte>> encode_reject(const Reject& reject);
Result<Reject> decode_reject(std::span<const std::byte> payload);

struct HandshakePolicy {
    std::vector<ProtocolVersion> supported_versions{ProtocolVersion{1, 0}};
    std::uint64_t supported_features = 0;
    std::uint64_t required_features = 0;
    SessionLimits limits;
    std::optional<std::string> join_token;
    std::optional<SessionRole> required_role;
};

} // namespace pluribus::fabric
