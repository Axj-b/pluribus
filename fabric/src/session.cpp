#include "pluribus/fabric/session.h"

#include "pluribus/fabric/message_type.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace pluribus::fabric {
namespace {

bool contains(
    const std::vector<ProtocolVersion>& versions,
    ProtocolVersion version) {
    return std::find(versions.begin(), versions.end(), version) != versions.end();
}

std::optional<ProtocolVersion> select_version(
    const std::vector<ProtocolVersion>& offered,
    const std::vector<ProtocolVersion>& supported) {
    std::optional<ProtocolVersion> selected;
    for (const auto version : offered) {
        if (!contains(supported, version)) {
            continue;
        }
        if (!selected || *selected < version) {
            selected = version;
        }
    }
    return selected;
}

bool tokens_equal(const std::string& left, const std::string& right) {
    if (left.size() != right.size()) {
        return false;
    }
    unsigned char difference = 0;
    for (std::size_t index = 0; index < left.size(); ++index) {
        difference |= static_cast<unsigned char>(
            static_cast<unsigned char>(left[index]) ^
            static_cast<unsigned char>(right[index]));
    }
    return difference == 0;
}

Result<void> send_bootstrap(
    FrameStream& stream,
    std::uint8_t type,
    std::vector<std::byte> payload) {
    Frame frame;
    frame.header.version = kBootstrapVersion;
    frame.header.type = type;
    frame.header.sequence = 0;
    frame.payload = std::move(payload);
    return stream.write_frame(frame);
}

Result<void> send_reject(
    FrameStream& stream,
    HandshakeRejectCode code,
    std::string diagnostic) {
    const auto encoded = encode_reject(Reject{code, std::move(diagnostic)});
    if (!encoded) {
        return std::unexpected(encoded.error());
    }
    return send_bootstrap(stream, kBootstrapRejectType, *encoded);
}

} // namespace

Result<Session> Session::accept(FrameStream stream, const HandshakePolicy& policy) {
    const auto received = stream.read_frame();
    if (!received) {
        return std::unexpected(received.error());
    }
    if (!received->has_value()) {
        return std::unexpected(
            make_error(ErrorCode::unavailable, "connection closed before hello"));
    }

    const Frame& frame = **received;
    if (frame.header.version != kBootstrapVersion ||
        frame.header.type != kBootstrapHelloType ||
        frame.header.sequence != 0) {
        (void)send_reject(
            stream,
            HandshakeRejectCode::invalid_argument,
            "expected a bootstrap Hello frame");
        return std::unexpected(make_error(
            ErrorCode::invalid_argument, "expected a bootstrap Hello frame"));
    }

    const auto hello = decode_hello(frame.payload);
    if (!hello) {
        (void)send_reject(
            stream,
            HandshakeRejectCode::invalid_argument,
            hello.error().diagnostic);
        return std::unexpected(hello.error());
    }

    if (policy.supported_versions.empty()) {
        (void)send_reject(
            stream,
            HandshakeRejectCode::internal,
            "server has no supported protocol versions");
        return std::unexpected(make_error(
            ErrorCode::internal, "server has no supported protocol versions"));
    }

    if (policy.required_role && hello->role != *policy.required_role) {
        (void)send_reject(
            stream,
            HandshakeRejectCode::unauthorized,
            "role is not permitted");
        return std::unexpected(
            make_error(ErrorCode::unauthorized, "role is not permitted"));
    }

    if (policy.join_token) {
        if (!hello->join_token || !tokens_equal(*policy.join_token, *hello->join_token)) {
            (void)send_reject(
                stream,
                HandshakeRejectCode::unauthorized,
                "join token is missing or invalid");
            return std::unexpected(make_error(
                ErrorCode::unauthorized, "join token is missing or invalid"));
        }
    }

    if ((hello->features & policy.required_features) != policy.required_features) {
        (void)send_reject(
            stream,
            HandshakeRejectCode::unsupported_feature,
            "required feature is not supported");
        return std::unexpected(make_error(
            ErrorCode::unsupported_feature, "required feature is not supported"));
    }

    const auto selected = select_version(hello->versions, policy.supported_versions);
    if (!selected) {
        (void)send_reject(
            stream,
            HandshakeRejectCode::unsupported_version,
            "no mutually supported protocol version");
        return std::unexpected(make_error(
            ErrorCode::unsupported_version, "no mutually supported protocol version"));
    }

    Welcome welcome;
    welcome.version = *selected;
    welcome.session = SessionId::generate();
    welcome.features = hello->features & policy.supported_features;
    welcome.limits = policy.limits;
    welcome.resumed = false;

    const auto encoded = encode_welcome(welcome);
    if (!encoded) {
        (void)send_reject(
            stream,
            HandshakeRejectCode::internal,
            encoded.error().diagnostic);
        return std::unexpected(encoded.error());
    }
    if (auto sent = send_bootstrap(stream, kBootstrapWelcomeType, *encoded); !sent) {
        return std::unexpected(sent.error());
    }

    return Session(
        std::move(stream),
        welcome.version,
        welcome.session,
        welcome.limits);
}

Result<Session> Session::connect(FrameStream stream, const Hello& hello) {
    const auto encoded = encode_hello(hello);
    if (!encoded) {
        return std::unexpected(encoded.error());
    }
    if (auto sent = send_bootstrap(stream, kBootstrapHelloType, *encoded); !sent) {
        return std::unexpected(sent.error());
    }

    const auto received = stream.read_frame();
    if (!received) {
        return std::unexpected(received.error());
    }
    if (!received->has_value()) {
        return std::unexpected(
            make_error(ErrorCode::unavailable, "connection closed before Welcome"));
    }

    const Frame& frame = **received;
    if (frame.header.version != kBootstrapVersion || frame.header.sequence != 0) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument, "invalid bootstrap response frame"));
    }

    if (frame.header.type == kBootstrapRejectType) {
        const auto reject = decode_reject(frame.payload);
        if (!reject) {
            return std::unexpected(reject.error());
        }
        return std::unexpected(
            make_error(to_error_code(reject->code), reject->diagnostic));
    }

    if (frame.header.type != kBootstrapWelcomeType) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument, "expected Welcome or Reject"));
    }

    const auto welcome = decode_welcome(frame.payload);
    if (!welcome) {
        return std::unexpected(welcome.error());
    }
    if (!contains(hello.versions, welcome->version)) {
        return std::unexpected(make_error(
            ErrorCode::unsupported_version,
            "server selected a version that was not offered"));
    }

    return Session(
        std::move(stream),
        welcome->version,
        welcome->session,
        welcome->limits);
}

Result<void> Session::send(const Frame& frame) {
    if (frame.payload.size() > limits_.max_control_payload) {
        return std::unexpected(make_error(
            ErrorCode::message_too_large,
            "frame payload exceeds the negotiated control limit"));
    }

    Frame outgoing = frame;
    outgoing.header.version = version_.major;
    outgoing.header.sequence = next_send_sequence_;
    auto written = stream_.write_frame(outgoing);
    if (!written) {
        return written;
    }
    ++next_send_sequence_;
    return {};
}

Result<std::optional<Frame>> Session::receive() {
    auto received = stream_.read_frame();
    if (!received) {
        return received;
    }
    if (!received->has_value()) {
        return received;
    }

    const Frame& frame = **received;
    if (frame.header.version != version_.major) {
        return std::unexpected(make_error(
            ErrorCode::unsupported_version,
            "received frame uses a different negotiated version"));
    }
    if (frame.header.sequence > next_receive_sequence_) {
        return std::unexpected(make_error(
            ErrorCode::sequence_gap,
            "received frame sequence is ahead of the expected sequence"));
    }
    if (frame.header.sequence < next_receive_sequence_) {
        return std::unexpected(make_error(
            ErrorCode::conflict,
            "received duplicate or out-of-order frame sequence"));
    }

    ++next_receive_sequence_;
    return received;
}

Result<void> Session::close() {
    return stream_.close();
}

} // namespace pluribus::fabric
