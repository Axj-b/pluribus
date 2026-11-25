#include "test.h"

#include <pluribus/fabric/frame.h>
#include <pluribus/fabric/frame_stream.h>
#include <pluribus/fabric/handshake.h>
#include <pluribus/fabric/session.h>
#include <pluribus/fabric/stream.h>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace pluribus;
using namespace pluribus::fabric;

Hello make_hello() {
    Hello hello;
    hello.role = SessionRole::client;
    hello.versions = {ProtocolVersion{1, 0}};
    hello.features = 0x1;
    hello.identity = "client-1";
    return hello;
}

HandshakePolicy make_policy() {
    HandshakePolicy policy;
    policy.supported_versions = {ProtocolVersion{1, 0}};
    policy.supported_features = 0x3;
    policy.limits = SessionLimits{64 * 1024, 4};
    policy.required_role = SessionRole::client;
    return policy;
}

Frame make_frame(std::uint8_t type, std::initializer_list<unsigned int> payload) {
    Frame frame;
    frame.header.type = type;
    for (const auto value : payload) {
        frame.payload.push_back(static_cast<std::byte>(value));
    }
    return frame;
}

} // namespace

PLURIBUS_TEST(fabric_handshake_codec_round_trip) {
    Hello hello = make_hello();
    hello.versions = {ProtocolVersion{1, 0}, ProtocolVersion{2, 1}};
    hello.join_token = "join-secret";
    hello.prior_session = *SessionId::parse("11111111-2222-4333-8444-555555555555");

    const auto encoded = encode_hello(hello);
    CHECK(encoded.has_value());
    const auto decoded = decode_hello(*encoded);
    CHECK(decoded.has_value());
    CHECK(decoded->role == hello.role);
    CHECK(decoded->versions == hello.versions);
    CHECK(decoded->features == hello.features);
    CHECK(decoded->identity == hello.identity);
    CHECK(decoded->join_token == hello.join_token);
    CHECK(decoded->prior_session == hello.prior_session);

    Welcome welcome;
    welcome.version = ProtocolVersion{1, 0};
    welcome.session = SessionId::generate();
    welcome.features = 0x2;
    welcome.limits = SessionLimits{4096, 2};
    welcome.resumed = true;
    const auto welcome_bytes = encode_welcome(welcome);
    CHECK(welcome_bytes.has_value());
    const auto welcome_decoded = decode_welcome(*welcome_bytes);
    CHECK(welcome_decoded.has_value());
    CHECK(welcome_decoded->version == welcome.version);
    CHECK(welcome_decoded->session == welcome.session);
    CHECK(welcome_decoded->features == welcome.features);
    CHECK(welcome_decoded->limits.max_control_payload ==
          welcome.limits.max_control_payload);
    CHECK(welcome_decoded->limits.max_outstanding_requests ==
          welcome.limits.max_outstanding_requests);
    CHECK(welcome_decoded->resumed);

    const Reject reject{HandshakeRejectCode::unauthorized, "bad token"};
    const auto reject_bytes = encode_reject(reject);
    CHECK(reject_bytes.has_value());
    const auto reject_decoded = decode_reject(*reject_bytes);
    CHECK(reject_decoded.has_value());
    CHECK(reject_decoded->code == reject.code);
    CHECK(reject_decoded->diagnostic == reject.diagnostic);
}

PLURIBUS_TEST(fabric_session_handshake_and_sequenced_exchange) {
    auto pair = make_loopback_streams();
    FrameStream server_stream(std::move(pair.second));
    FrameStream client_stream(std::move(pair.first));

    const auto policy = make_policy();
    std::string server_error;
    SessionId server_session;
    std::thread server([&] {
        try {
            auto accepted = Session::accept(std::move(server_stream), policy);
            if (!accepted) {
                server_error = accepted.error().diagnostic;
                return;
            }
            auto session = std::move(*accepted);
            server_session = session.id();

            for (int index = 0; index < 2; ++index) {
                const auto incoming = session.receive();
                if (!incoming) {
                    server_error = incoming.error().diagnostic;
                    return;
                }
                if (!incoming->has_value()) {
                    server_error = "server saw EOF before a frame";
                    return;
                }
                const auto written = session.send(**incoming);
                if (!written) {
                    server_error = written.error().diagnostic;
                    return;
                }
            }
            (void)session.close();
        } catch (const std::exception& exception) {
            server_error = exception.what();
        }
    });

    const Hello hello = make_hello();
    auto connected = Session::connect(std::move(client_stream), hello);
    CHECK(connected.has_value());
    auto session = std::move(*connected);
    CHECK(!session.id().is_nil());
    CHECK((session.version() == ProtocolVersion{1, 0}));
    CHECK(session.limits().max_control_payload == policy.limits.max_control_payload);

    const Frame first = make_frame(100, {0x01, 0x02});
    const Frame second = make_frame(101, {0x03});
    CHECK(session.send(first).has_value());
    CHECK(session.send(second).has_value());

    const auto first_response = session.receive();
    CHECK(first_response.has_value());
    CHECK(first_response->has_value());
    CHECK((*first_response)->header.type == first.header.type);
    CHECK((*first_response)->payload == first.payload);

    const auto second_response = session.receive();
    CHECK(second_response.has_value());
    CHECK(second_response->has_value());
    CHECK((*second_response)->header.type == second.header.type);
    CHECK((*second_response)->payload == second.payload);

    server.join();
    CHECK(server_error.empty());
    CHECK(server_session == session.id());
    CHECK(session.close().has_value());
}

PLURIBUS_TEST(fabric_session_rejects_unsupported_version) {
    auto pair = make_loopback_streams();
    FrameStream server_stream(std::move(pair.second));
    FrameStream client_stream(std::move(pair.first));

    HandshakePolicy policy;
    policy.supported_versions = {ProtocolVersion{2, 0}};

    std::string server_error;
    std::thread server([&] {
        try {
            auto accepted = Session::accept(std::move(server_stream), policy);
            if (!accepted) {
                server_error = accepted.error().diagnostic;
            }
        } catch (const std::exception& exception) {
            server_error = exception.what();
        }
    });

    const auto connected = Session::connect(std::move(client_stream), make_hello());
    CHECK(!connected.has_value());
    CHECK(connected.error().code == ErrorCode::unsupported_version);

    server.join();
    CHECK(!server_error.empty());
}

PLURIBUS_TEST(fabric_session_rejects_bad_join_token) {
    auto pair = make_loopback_streams();
    FrameStream server_stream(std::move(pair.second));
    FrameStream client_stream(std::move(pair.first));

    auto policy = make_policy();
    policy.join_token = "expected";

    std::string server_error;
    std::thread server([&] {
        try {
            auto accepted = Session::accept(std::move(server_stream), policy);
            if (!accepted) {
                server_error = accepted.error().diagnostic;
            }
        } catch (const std::exception& exception) {
            server_error = exception.what();
        }
    });

    auto hello = make_hello();
    hello.join_token = "wrong";
    const auto connected = Session::connect(std::move(client_stream), hello);
    CHECK(!connected.has_value());
    CHECK(connected.error().code == ErrorCode::unauthorized);

    server.join();
    CHECK(!server_error.empty());
}
