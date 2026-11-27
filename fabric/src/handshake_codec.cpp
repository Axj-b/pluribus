#include "pluribus/fabric/handshake.h"

#include "handshake.pb.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace pluribus::fabric {
namespace {

constexpr std::size_t kMaxIdentityBytes = 128;
constexpr std::size_t kMaxJoinTokenBytes = 4096;
constexpr std::size_t kMaxRejectDiagnosticBytes = 1024;
constexpr std::size_t kMaxVersions = 8;
constexpr std::size_t kSessionIdBytes = 16;

Error invalid(std::string diagnostic) {
    return make_error(ErrorCode::invalid_argument, std::move(diagnostic));
}

wire::SessionRole to_wire_role(SessionRole role) {
    return role == SessionRole::node ? wire::SESSION_ROLE_NODE : wire::SESSION_ROLE_CLIENT;
}

SessionRole from_wire_role(wire::SessionRole role) {
    return role == wire::SESSION_ROLE_NODE ? SessionRole::node : SessionRole::client;
}

bool serialize(const google::protobuf::MessageLite& message, std::vector<std::byte>& output) {
    std::string bytes;
    if (!message.SerializeToString(&bytes)) {
        return false;
    }
    output.clear();
    output.reserve(bytes.size());
    for (const char byte : bytes) {
        output.push_back(static_cast<std::byte>(static_cast<unsigned char>(byte)));
    }
    return true;
}

Error serialization_failed() {
    return make_error(ErrorCode::internal, "protobuf serialization failed");
}

Result<std::vector<std::byte>> finish(const google::protobuf::MessageLite& message) {
    std::vector<std::byte> output;
    if (!serialize(message, output)) {
        return std::unexpected(serialization_failed());
    }
    return output;
}

bool valid_role(SessionRole role) noexcept {
    return role == SessionRole::node || role == SessionRole::client;
}

} // namespace

ErrorCode to_error_code(HandshakeRejectCode code) noexcept {
    switch (code) {
    case HandshakeRejectCode::unsupported_version:
        return ErrorCode::unsupported_version;
    case HandshakeRejectCode::unsupported_feature:
        return ErrorCode::unsupported_feature;
    case HandshakeRejectCode::unauthorized:
        return ErrorCode::unauthorized;
    case HandshakeRejectCode::invalid_argument:
        return ErrorCode::invalid_argument;
    case HandshakeRejectCode::internal:
        return ErrorCode::internal;
    }
    return ErrorCode::internal;
}

Result<std::vector<std::byte>> encode_hello(const Hello& hello) {
    if (!valid_role(hello.role)) {
        return std::unexpected(invalid("hello role is invalid"));
    }
    if (hello.versions.empty() || hello.versions.size() > kMaxVersions) {
        return std::unexpected(invalid("hello version list is empty or too large"));
    }
    for (const auto& version : hello.versions) {
        if (version.major == 0) {
            return std::unexpected(invalid("bootstrap version cannot be advertised"));
        }
    }
    if (hello.identity.empty() || hello.identity.size() > kMaxIdentityBytes) {
        return std::unexpected(invalid("hello identity is empty or too long"));
    }
    if (hello.join_token && hello.join_token->size() > kMaxJoinTokenBytes) {
        return std::unexpected(invalid("hello join token is too long"));
    }
    if (hello.prior_session && hello.prior_session->is_nil()) {
        return std::unexpected(invalid("hello prior session is nil"));
    }

    wire::Hello message;
    message.set_role(to_wire_role(hello.role));
    for (const auto& version : hello.versions) {
        auto* wire_version = message.add_versions();
        wire_version->set_major(version.major);
        wire_version->set_minor(version.minor);
    }
    message.set_features(hello.features);
    message.set_identity(hello.identity);
    if (hello.join_token) {
        message.set_join_token(*hello.join_token);
    }
    if (hello.prior_session) {
        const auto bytes = hello.prior_session->value().bytes();
        message.set_prior_session(
            std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
    }
    return finish(message);
}

Result<Hello> decode_hello(std::span<const std::byte> payload) {
    wire::Hello message;
    if (!message.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        return std::unexpected(invalid("hello payload is not a valid protobuf message"));
    }

    if (message.role() != wire::SESSION_ROLE_NODE &&
        message.role() != wire::SESSION_ROLE_CLIENT) {
        return std::unexpected(invalid("hello role is invalid"));
    }
    if (message.versions_size() == 0 ||
        message.versions_size() > static_cast<int>(kMaxVersions)) {
        return std::unexpected(invalid("hello version list is empty or too large"));
    }
    if (message.identity().empty() || message.identity().size() > kMaxIdentityBytes) {
        return std::unexpected(invalid("hello identity is empty or too long"));
    }
    if (message.join_token().size() > kMaxJoinTokenBytes) {
        return std::unexpected(invalid("hello join token is too long"));
    }
    if (!message.prior_session().empty() &&
        message.prior_session().size() != kSessionIdBytes) {
        return std::unexpected(invalid("hello prior session is not a 16-byte UUID"));
    }

    Hello hello;
    hello.role = from_wire_role(message.role());
    hello.versions.reserve(static_cast<std::size_t>(message.versions_size()));
    for (const auto& version : message.versions()) {
        if (version.major() == 0) {
            return std::unexpected(invalid("bootstrap version cannot be advertised"));
        }
        hello.versions.push_back(ProtocolVersion{
            static_cast<std::uint8_t>(version.major()),
            static_cast<std::uint8_t>(version.minor())});
    }
    hello.features = message.features();
    hello.identity = message.identity();
    if (!message.join_token().empty()) {
        hello.join_token = message.join_token();
    }
    if (!message.prior_session().empty()) {
        const auto& raw = message.prior_session();
        Uuid::Bytes bytes{};
        for (std::size_t index = 0; index < bytes.size(); ++index) {
            bytes[index] = static_cast<std::uint8_t>(raw[static_cast<std::size_t>(index)]);
        }
        hello.prior_session = SessionId(Uuid(bytes));
    }
    return hello;
}

Result<std::vector<std::byte>> encode_welcome(const Welcome& welcome) {
    if (welcome.version.major == 0) {
        return std::unexpected(invalid("welcome version cannot be bootstrap"));
    }
    if (welcome.session.is_nil()) {
        return std::unexpected(invalid("welcome session ID is nil"));
    }
    if (welcome.limits.max_control_payload == 0 ||
        welcome.limits.max_control_payload >
            static_cast<std::uint32_t>(kMaxFrameLength - kHeaderBytes)) {
        return std::unexpected(invalid("welcome control payload limit is invalid"));
    }
    if (welcome.limits.max_outstanding_requests == 0 ||
        welcome.limits.max_outstanding_requests > 1024) {
        return std::unexpected(invalid("welcome outstanding-request limit is invalid"));
    }

    wire::Welcome message;
    message.mutable_version()->set_major(welcome.version.major);
    message.mutable_version()->set_minor(welcome.version.minor);
    const auto bytes = welcome.session.value().bytes();
    message.set_session(
        std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
    message.set_features(welcome.features);
    message.set_max_control_payload(welcome.limits.max_control_payload);
    message.set_max_outstanding_requests(welcome.limits.max_outstanding_requests);
    message.set_resumed(welcome.resumed);
    return finish(message);
}

Result<Welcome> decode_welcome(std::span<const std::byte> payload) {
    wire::Welcome message;
    if (!message.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        return std::unexpected(invalid("welcome payload is not a valid protobuf message"));
    }
    if (!message.has_version() || message.version().major() == 0) {
        return std::unexpected(invalid("welcome version is missing or bootstrap"));
    }
    if (message.session().size() != kSessionIdBytes) {
        return std::unexpected(invalid("welcome session is not a 16-byte UUID"));
    }
    if (message.max_control_payload() == 0 ||
        message.max_control_payload() >
            static_cast<std::uint32_t>(kMaxFrameLength - kHeaderBytes)) {
        return std::unexpected(invalid("welcome control payload limit is invalid"));
    }
    if (message.max_outstanding_requests() == 0 ||
        message.max_outstanding_requests() > 1024) {
        return std::unexpected(invalid("welcome outstanding-request limit is invalid"));
    }

    const auto& raw = message.session();
    Uuid::Bytes bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::uint8_t>(raw[static_cast<std::size_t>(index)]);
    }

    Welcome welcome;
    welcome.version = ProtocolVersion{
        static_cast<std::uint8_t>(message.version().major()),
        static_cast<std::uint8_t>(message.version().minor())};
    welcome.session = SessionId(Uuid(bytes));
    welcome.features = message.features();
    welcome.limits = SessionLimits{
        message.max_control_payload(),
        message.max_outstanding_requests()};
    welcome.resumed = message.resumed();
    return welcome;
}

Result<std::vector<std::byte>> encode_reject(const Reject& reject) {
    if (reject.diagnostic.size() > kMaxRejectDiagnosticBytes) {
        return std::unexpected(invalid("reject diagnostic is too long"));
    }

    wire::Reject message;
    switch (reject.code) {
    case HandshakeRejectCode::unsupported_version:
        message.set_code(wire::REJECT_CODE_UNSUPPORTED_VERSION);
        break;
    case HandshakeRejectCode::unsupported_feature:
        message.set_code(wire::REJECT_CODE_UNSUPPORTED_FEATURE);
        break;
    case HandshakeRejectCode::unauthorized:
        message.set_code(wire::REJECT_CODE_UNAUTHORIZED);
        break;
    case HandshakeRejectCode::invalid_argument:
        message.set_code(wire::REJECT_CODE_INVALID_ARGUMENT);
        break;
    case HandshakeRejectCode::internal:
        message.set_code(wire::REJECT_CODE_INTERNAL);
        break;
    }
    message.set_diagnostic(reject.diagnostic);
    return finish(message);
}

Result<Reject> decode_reject(std::span<const std::byte> payload) {
    wire::Reject message;
    if (!message.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        return std::unexpected(invalid("reject payload is not a valid protobuf message"));
    }
    if (message.diagnostic().size() > kMaxRejectDiagnosticBytes) {
        return std::unexpected(invalid("reject diagnostic is too long"));
    }

    Reject reject;
    switch (message.code()) {
    case wire::REJECT_CODE_UNSUPPORTED_VERSION:
        reject.code = HandshakeRejectCode::unsupported_version;
        break;
    case wire::REJECT_CODE_UNSUPPORTED_FEATURE:
        reject.code = HandshakeRejectCode::unsupported_feature;
        break;
    case wire::REJECT_CODE_UNAUTHORIZED:
        reject.code = HandshakeRejectCode::unauthorized;
        break;
    case wire::REJECT_CODE_INVALID_ARGUMENT:
        reject.code = HandshakeRejectCode::invalid_argument;
        break;
    case wire::REJECT_CODE_INTERNAL:
        reject.code = HandshakeRejectCode::internal;
        break;
    default:
        return std::unexpected(invalid("reject code is invalid"));
    }
    reject.diagnostic = message.diagnostic();
    return reject;
}

} // namespace pluribus::fabric
