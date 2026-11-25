#include "test.h"

#include <pluribus/fabric/frame.h>
#include <pluribus/fabric/frame_stream.h>
#include <pluribus/fabric/stream.h>
#include <pluribus/fabric/tcp.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <memory>
#include <span>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace pluribus;
using namespace pluribus::fabric;

Frame make_frame(
    std::uint8_t type,
    std::uint64_t sequence,
    std::initializer_list<unsigned int> payload) {
    Frame frame;
    frame.header.version = kProtocolVersion;
    frame.header.type = type;
    frame.header.sequence = sequence;
    for (const auto value : payload) {
        frame.payload.push_back(static_cast<std::byte>(value));
    }
    return frame;
}

} // namespace

PLURIBUS_TEST(fabric_loopback_stream_round_trip) {
    auto pair = make_loopback_streams();
    CHECK(pair.first != nullptr);
    CHECK(pair.second != nullptr);

    const std::array<std::byte, 5> message{
        std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
    CHECK(pair.first->write(message).has_value());

    std::array<std::byte, 5> received{};
    const auto count = pair.second->read(received);
    CHECK(count.has_value());
    CHECK(*count == message.size());
    CHECK(received == message);

    CHECK(pair.first->close().has_value());
    const auto eof = pair.second->read(received);
    CHECK(eof.has_value());
    CHECK(*eof == 0);
    CHECK(pair.second->close().has_value());
}

PLURIBUS_TEST(fabric_frame_stream_over_loopback) {
    auto pair = make_loopback_streams();
    FrameStream first(std::move(pair.first));
    FrameStream second(std::move(pair.second));

    const Frame request = make_frame(42, 1, {0xDE, 0xAD, 0xBE, 0xEF});
    CHECK(first.write_frame(request).has_value());

    const auto received = second.read_frame();
    CHECK(received.has_value());
    CHECK(received->has_value());
    CHECK((*received)->header == request.header);
    CHECK((*received)->payload == request.payload);

    const Frame response = make_frame(43, 2, {0xCA, 0xFE});
    CHECK(second.write_frame(response).has_value());

    const auto reply = first.read_frame();
    CHECK(reply.has_value());
    CHECK(reply->has_value());
    CHECK((*reply)->header == response.header);
    CHECK((*reply)->payload == response.payload);

    CHECK(first.close().has_value());
    const auto end = second.read_frame();
    CHECK(end.has_value());
    CHECK(!end->has_value());
    CHECK(second.close().has_value());
}

PLURIBUS_TEST(fabric_frame_stream_detects_truncated_frame) {
    auto pair = make_loopback_streams();
    const auto encoded = encode_frame(make_frame(1, 1, {0x01, 0x02, 0x03}));
    CHECK(encoded.has_value());

    CHECK(pair.first
              ->write(std::span<const std::byte>(*encoded).first(encoded->size() - 1))
              .has_value());
    CHECK(pair.first->close().has_value());

    FrameStream second(std::move(pair.second));
    const auto result = second.read_frame();
    CHECK(!result.has_value());
    CHECK(result.error().code == ErrorCode::invalid_argument);
}

PLURIBUS_TEST(fabric_tcp_loopback_exchanges_frames) {
    const auto listener = TcpListener::bind("127.0.0.1", 0);
    CHECK(listener.has_value());

    auto client = connect_tcp("127.0.0.1", (*listener)->port());
    CHECK(client.has_value());

    const Frame request = make_frame(10, 1, {0x01, 0x02, 0x03});
    const Frame response = make_frame(11, 2, {0x04, 0x05});

    std::string server_error;
    std::thread server([&] {
        try {
            auto accepted = (*listener)->accept();
            if (!accepted) {
                server_error = accepted.error().diagnostic;
                return;
            }

            FrameStream stream(std::move(*accepted));
            const auto incoming = stream.read_frame();
            if (!incoming) {
                server_error = incoming.error().diagnostic;
                (void)stream.close();
                return;
            }
            if (!incoming->has_value()) {
                server_error = "server saw EOF before a frame";
                (void)stream.close();
                return;
            }
            if ((*incoming)->header != request.header ||
                (*incoming)->payload != request.payload) {
                server_error = "server received a different frame";
                (void)stream.close();
                return;
            }

            const auto written = stream.write_frame(response);
            if (!written) {
                server_error = written.error().diagnostic;
            }
            (void)stream.close();
        } catch (const std::exception& exception) {
            server_error = exception.what();
        }
    });

    FrameStream client_stream(std::move(*client));
    const auto written = client_stream.write_frame(request);
    CHECK(written.has_value());

    const auto received = client_stream.read_frame();
    CHECK(received.has_value());
    CHECK(received->has_value());
    CHECK((*received)->header == response.header);
    CHECK((*received)->payload == response.payload);

    server.join();
    CHECK(server_error.empty());
    CHECK(client_stream.close().has_value());
    CHECK((*listener)->close().has_value());
}

PLURIBUS_TEST(fabric_tcp_rejects_invalid_address) {
    const auto result = TcpListener::bind("not-an-ip", 0);
    CHECK(!result.has_value());
    CHECK(result.error().code == ErrorCode::invalid_argument);
}
