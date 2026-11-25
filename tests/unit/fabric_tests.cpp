#include "test.h"

#include <pluribus/fabric/frame.h>
#include <pluribus/fabric/message_type.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

using namespace pluribus;
using namespace pluribus::fabric;

std::vector<std::byte> payload(std::initializer_list<unsigned int> values) {
    std::vector<std::byte> result;
    result.reserve(values.size());
    for (const auto value : values) {
        result.push_back(static_cast<std::byte>(value));
    }
    return result;
}

Frame make_frame() {
    Frame frame;
    frame.header.version = kProtocolVersion;
    frame.header.type = 42;
    frame.header.flags = 0;
    frame.header.sequence = 0x0102030405060708ULL;
    frame.payload = payload({0xDE, 0xAD});
    return frame;
}

std::uint32_t read_u32(const std::vector<std::byte>& bytes, std::size_t offset) {
    return (std::to_integer<std::uint32_t>(bytes[offset]) << 24) |
           (std::to_integer<std::uint32_t>(bytes[offset + 1]) << 16) |
           (std::to_integer<std::uint32_t>(bytes[offset + 2]) << 8) |
           std::to_integer<std::uint32_t>(bytes[offset + 3]);
}

std::uint8_t read_u8(const std::vector<std::byte>& bytes, std::size_t offset) {
    return std::to_integer<std::uint8_t>(bytes[offset]);
}

} // namespace

PLURIBUS_TEST(fabric_frame_round_trip_and_network_byte_order) {
    const Frame original = make_frame();
    const auto encoded = encode_frame(original);
    CHECK(encoded.has_value());
    CHECK(encoded->size() == 4 + 12 + 2);

    CHECK(read_u32(*encoded, 0) == 14);
    CHECK(read_u8(*encoded, 4) == kProtocolVersion);
    CHECK(read_u8(*encoded, 5) == 42);
    CHECK(read_u8(*encoded, 6) == 0x00);
    CHECK(read_u8(*encoded, 7) == 0x00);
    CHECK(read_u8(*encoded, 8) == 0x01);
    CHECK(read_u8(*encoded, 9) == 0x02);
    CHECK(read_u8(*encoded, 10) == 0x03);
    CHECK(read_u8(*encoded, 11) == 0x04);
    CHECK(read_u8(*encoded, 12) == 0x05);
    CHECK(read_u8(*encoded, 13) == 0x06);
    CHECK(read_u8(*encoded, 14) == 0x07);
    CHECK(read_u8(*encoded, 15) == 0x08);
    CHECK(read_u8(*encoded, 16) == 0xDE);
    CHECK(read_u8(*encoded, 17) == 0xAD);

    const auto decoded = decode_frame(*encoded);
    CHECK(decoded.has_value());
    CHECK(decoded->header == original.header);
    CHECK(decoded->payload == original.payload);
}

PLURIBUS_TEST(fabric_frame_accepts_empty_payload) {
    Frame frame;
    frame.header.version = kProtocolVersion;
    const auto encoded = encode_frame(frame);
    CHECK(encoded.has_value());
    CHECK(read_u32(*encoded, 0) == 12);

    const auto decoded = decode_frame(*encoded);
    CHECK(decoded.has_value());
    CHECK(decoded->payload.empty());
}

PLURIBUS_TEST(fabric_frame_enforces_control_message_limit) {
    const std::size_t max_payload = kMaxFrameLength - kHeaderBytes;
    Frame frame;
    frame.header.version = kProtocolVersion;
    frame.payload.assign(max_payload, std::byte{0x5A});

    const auto encoded = encode_frame(frame);
    CHECK(encoded.has_value());
    CHECK(encoded->size() == static_cast<std::size_t>(kMaxFrameLength) + 4);

    const auto decoded = decode_frame(*encoded);
    CHECK(decoded.has_value());
    CHECK(decoded->payload.size() == max_payload);

    frame.payload.push_back(std::byte{0x00});
    const auto too_large = encode_frame(frame);
    CHECK(!too_large.has_value());
    CHECK(too_large.error().code == ErrorCode::message_too_large);
}

PLURIBUS_TEST(fabric_frame_rejects_invalid_lengths_and_truncation) {
    const auto valid = encode_frame(make_frame());
    CHECK(valid.has_value());

    CHECK(!decode_frame(std::span<const std::byte>{}).has_value());

    auto below_minimum = *valid;
    below_minimum[3] = std::byte{11};
    auto result = decode_frame(below_minimum);
    CHECK(!result.has_value());
    CHECK(result.error().code == ErrorCode::invalid_argument);

    auto above_maximum = *valid;
    above_maximum[0] = std::byte{0xFF};
    above_maximum[1] = std::byte{0xFF};
    above_maximum[2] = std::byte{0xFF};
    above_maximum[3] = std::byte{0xFF};
    result = decode_frame(above_maximum);
    CHECK(!result.has_value());
    CHECK(result.error().code == ErrorCode::message_too_large);

    auto truncated = *valid;
    truncated.pop_back();
    result = decode_frame(truncated);
    CHECK(!result.has_value());
    CHECK(result.error().code == ErrorCode::invalid_argument);

    auto with_trailing_byte = *valid;
    with_trailing_byte.push_back(std::byte{0x00});
    result = decode_frame(with_trailing_byte);
    CHECK(!result.has_value());
    CHECK(result.error().code == ErrorCode::invalid_argument);
}

PLURIBUS_TEST(fabric_frame_validates_version_and_flags) {
    Frame frame;
    frame.header.version = kProtocolVersion;
    frame.header.flags = 1;

    auto encoded = encode_frame(frame);
    CHECK(!encoded.has_value());
    CHECK(encoded.error().code == ErrorCode::unsupported_feature);

    FramingLimits limits;
    limits.known_flags = 1;
    encoded = encode_frame(frame, limits);
    CHECK(encoded.has_value());

    const auto decoded = decode_frame(*encoded, limits);
    CHECK(decoded.has_value());

    const auto rejected = decode_frame(*encoded);
    CHECK(!rejected.has_value());
    CHECK(rejected.error().code == ErrorCode::unsupported_feature);

    frame.header.flags = 0;
    frame.header.version = 2;
    encoded = encode_frame(frame);
    CHECK(!encoded.has_value());
    CHECK(encoded.error().code == ErrorCode::unsupported_version);

    frame.header.version = kBootstrapVersion;
    encoded = encode_frame(frame);
    CHECK(encoded.has_value());
}

PLURIBUS_TEST(fabric_decoder_handles_arbitrary_chunking) {
    const auto encoded = encode_frame(make_frame());
    CHECK(encoded.has_value());

    FrameDecoder decoder;
    for (std::size_t index = 0; index < encoded->size(); ++index) {
        const auto pushed = decoder.push(std::span<const std::byte>(*encoded).subspan(index, 1));
        CHECK(pushed.has_value());

        auto next = decoder.next();
        CHECK(next.has_value());
        if (index + 1 < encoded->size()) {
            CHECK(!next->has_value());
        } else {
            CHECK(next->has_value());
            CHECK((*next)->header == make_frame().header);
            CHECK((*next)->payload == make_frame().payload);
        }
    }

    auto end = decoder.next();
    CHECK(end.has_value());
    CHECK(!end->has_value());
}

PLURIBUS_TEST(fabric_decoder_emits_multiple_frames_in_order) {
    Frame first;
    first.header.version = kProtocolVersion;
    first.header.type = 1;
    first.header.sequence = 1;
    Frame second;
    second.header.version = kProtocolVersion;
    second.header.type = 2;
    second.header.sequence = 2;
    second.payload = payload({0x01, 0x02, 0x03});

    const auto first_bytes = encode_frame(first);
    const auto second_bytes = encode_frame(second);
    CHECK(first_bytes.has_value());
    CHECK(second_bytes.has_value());

    std::vector<std::byte> combined = *first_bytes;
    combined.insert(combined.end(), second_bytes->begin(), second_bytes->end());

    FrameDecoder decoder;
    CHECK(decoder.push(combined).has_value());

    auto one = decoder.next();
    CHECK(one.has_value());
    CHECK(one->has_value());
    CHECK((*one)->header == first.header);

    auto two = decoder.next();
    CHECK(two.has_value());
    CHECK(two->has_value());
    CHECK((*two)->header == second.header);
    CHECK((*two)->payload == second.payload);

    auto three = decoder.next();
    CHECK(three.has_value());
    CHECK(!three->has_value());
}

PLURIBUS_TEST(fabric_decoder_handles_partial_trailing_frame) {
    Frame first;
    first.header.version = kProtocolVersion;
    first.header.sequence = 1;
    Frame second;
    second.header.version = kProtocolVersion;
    second.header.sequence = 2;

    const auto first_bytes = encode_frame(first);
    const auto second_bytes = encode_frame(second);
    CHECK(first_bytes.has_value());
    CHECK(second_bytes.has_value());

    FrameDecoder decoder;
    CHECK(decoder.push(*first_bytes).has_value());
    CHECK(decoder.push(std::span<const std::byte>(*second_bytes).first(5)).has_value());

    auto one = decoder.next();
    CHECK(one.has_value());
    CHECK(one->has_value());
    CHECK((*one)->header.sequence == 1);

    auto two = decoder.next();
    CHECK(two.has_value());
    CHECK(!two->has_value());

    CHECK(decoder.push(std::span<const std::byte>(*second_bytes).subspan(5)).has_value());
    two = decoder.next();
    CHECK(two.has_value());
    CHECK(two->has_value());
    CHECK((*two)->header.sequence == 2);
}

PLURIBUS_TEST(fabric_decoder_latches_framing_errors) {
    std::vector<std::byte> invalid_length(4, std::byte{0});
    invalid_length[3] = std::byte{5};

    FrameDecoder decoder;
    CHECK(decoder.push(invalid_length).has_value());

    auto result = decoder.next();
    CHECK(!result.has_value());
    CHECK(result.error().code == ErrorCode::invalid_argument);
    CHECK(decoder.failed());

    const auto valid = encode_frame(Frame{});
    CHECK(valid.has_value());
    const auto pushed_after_failure = decoder.push(*valid);
    CHECK(!pushed_after_failure.has_value());

    auto next_after_failure = decoder.next();
    CHECK(!next_after_failure.has_value());
}

PLURIBUS_TEST(fabric_bootstrap_type_classification) {
    CHECK(is_bootstrap_message_type(kBootstrapHelloType));
    CHECK(is_bootstrap_message_type(kBootstrapWelcomeType));
    CHECK(is_bootstrap_message_type(kBootstrapRejectType));
    CHECK(!is_bootstrap_message_type(0));
    CHECK(!is_bootstrap_message_type(4));
}
