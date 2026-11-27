#include "test.h"

#include <pluribus/core/version.h>
#include <pluribus/fabric/control.h>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

namespace {

using namespace pluribus;
using namespace pluribus::fabric;

RequestId make_request_id() {
    return *RequestId::parse("11111111-2222-4333-8444-555555555555");
}

NodeId make_node_id() {
    return *NodeId::parse("aaaaaaaa-bbbb-4ccc-8ddd-eeeeeeeeeeee");
}

NodeDescriptor make_descriptor(CpuArchitecture architecture = CpuArchitecture::x86_64) {
    NodeDescriptor descriptor;
    descriptor.id = make_node_id();
    descriptor.name = "node-alpha";
    descriptor.protocol_major = 1;
    descriptor.abi_version = *AbiVersion::parse("pluribus-v1");
    descriptor.architecture = architecture;
    descriptor.logical_cpu_count = 8;
    descriptor.total_memory_bytes = 16ull * 1024 * 1024 * 1024;
    descriptor.capabilities.insert(*CapabilityName::parse("gpu.cuda"));
    descriptor.capabilities.insert(*CapabilityName::parse("compute.fp32"));
    descriptor.devices.insert(*CapabilityName::parse("device.radar.front"));
    return descriptor;
}

void check_descriptor(const NodeDescriptor& expected, const NodeDescriptor& actual) {
    CHECK(actual.id == expected.id);
    CHECK(actual.name == expected.name);
    CHECK(actual.protocol_major == expected.protocol_major);
    CHECK(actual.abi_version == expected.abi_version);
    CHECK(actual.architecture == expected.architecture);
    CHECK(actual.logical_cpu_count == expected.logical_cpu_count);
    CHECK(actual.total_memory_bytes == expected.total_memory_bytes);
    CHECK(actual.capabilities == expected.capabilities);
    CHECK(actual.devices == expected.devices);
}

std::vector<std::byte> raw_bytes(std::initializer_list<unsigned int> values) {
    std::vector<std::byte> out;
    out.reserve(values.size());
    for (const auto value : values) {
        out.push_back(static_cast<std::byte>(value));
    }
    return out;
}

// Protobuf Envelope field 1 (request_id): tag 0x0A, 16-byte length.
std::vector<std::byte> request_id_field(bool nil) {
    std::vector<std::byte> out{std::byte{0x0A}, std::byte{0x10}};
    for (std::uint8_t index = 0; index < 16; ++index) {
        out.push_back(static_cast<std::byte>(nil ? 0 : index + 1));
    }
    return out;
}

} // namespace

PLURIBUS_TEST(fabric_control_register_node_round_trip_x86_64) {
    ControlMessage message;
    message.request_id = make_request_id();
    message.body = RegisterNode{make_descriptor(CpuArchitecture::x86_64), 7};

    const auto encoded = encode_control(static_cast<std::uint8_t>(MessageType::register_node),
                                        message);
    CHECK(encoded.has_value());
    const auto decoded = decode_control(*encoded,
                                        static_cast<std::uint8_t>(MessageType::register_node));
    CHECK(decoded.has_value());
    CHECK(decoded->request_id == message.request_id);
    const auto* body = std::get_if<RegisterNode>(&decoded->body);
    CHECK(body != nullptr);
    CHECK(body->incarnation == 7);
    check_descriptor(std::get<RegisterNode>(message.body).descriptor, body->descriptor);
}

PLURIBUS_TEST(fabric_control_register_node_round_trip_arm64) {
    ControlMessage message;
    message.request_id = make_request_id();
    message.body = RegisterNode{make_descriptor(CpuArchitecture::arm64), 1};

    const auto encoded = encode_control(static_cast<std::uint8_t>(MessageType::register_node),
                                        message);
    CHECK(encoded.has_value());
    const auto decoded = decode_control(*encoded,
                                        static_cast<std::uint8_t>(MessageType::register_node));
    CHECK(decoded.has_value());
    const auto* body = std::get_if<RegisterNode>(&decoded->body);
    CHECK(body != nullptr);
    CHECK(body->descriptor.architecture == CpuArchitecture::arm64);
    check_descriptor(std::get<RegisterNode>(message.body).descriptor, body->descriptor);
}

PLURIBUS_TEST(fabric_control_register_node_ack_round_trip) {
    ControlMessage message;
    message.request_id = make_request_id();
    message.body = RegisterNodeAck{250, true};

    const auto encoded = encode_control(
        static_cast<std::uint8_t>(MessageType::register_node_ack), message);
    CHECK(encoded.has_value());
    const auto decoded = decode_control(
        *encoded, static_cast<std::uint8_t>(MessageType::register_node_ack));
    CHECK(decoded.has_value());
    CHECK(decoded->request_id == message.request_id);
    const auto* body = std::get_if<RegisterNodeAck>(&decoded->body);
    CHECK(body != nullptr);
    CHECK(body->heartbeat_interval_ms == 250);
    CHECK(body->reconciliation_required);
}

PLURIBUS_TEST(fabric_control_heartbeat_round_trip) {
    ControlMessage message;
    message.body = Heartbeat{make_node_id(), 3, 42, 9, NodeHealth::degraded};

    const auto encoded =
        encode_control(static_cast<std::uint8_t>(MessageType::heartbeat), message);
    CHECK(encoded.has_value());
    const auto decoded =
        decode_control(*encoded, static_cast<std::uint8_t>(MessageType::heartbeat));
    CHECK(decoded.has_value());
    CHECK(!decoded->request_id.has_value());
    const auto* body = std::get_if<Heartbeat>(&decoded->body);
    CHECK(body != nullptr);
    CHECK(body->node_id == make_node_id());
    CHECK(body->incarnation == 3);
    CHECK(body->heartbeat_seq == 42);
    CHECK(body->descriptor_revision == 9);
    CHECK(body->health == NodeHealth::degraded);
}

PLURIBUS_TEST(fabric_control_heartbeat_ack_round_trip) {
    ControlMessage message;
    message.body = HeartbeatAck{99};

    const auto encoded =
        encode_control(static_cast<std::uint8_t>(MessageType::heartbeat_ack), message);
    CHECK(encoded.has_value());
    const auto decoded =
        decode_control(*encoded, static_cast<std::uint8_t>(MessageType::heartbeat_ack));
    CHECK(decoded.has_value());
    CHECK(!decoded->request_id.has_value());
    const auto* body = std::get_if<HeartbeatAck>(&decoded->body);
    CHECK(body != nullptr);
    CHECK(body->heartbeat_seq == 99);
}

PLURIBUS_TEST(fabric_control_error_payload_on_ack_types) {
    Error error;
    error.code = ErrorCode::unavailable;
    error.diagnostic = "node is draining";
    error.component = "master.registry";
    error.node = make_node_id();
    error.execution = *ExecutionId::parse("99999999-8888-4777-8666-555555555555");

    for (const auto type : {static_cast<std::uint8_t>(MessageType::register_node_ack),
                            static_cast<std::uint8_t>(MessageType::heartbeat_ack)}) {
        ControlMessage message;
        if (type == static_cast<std::uint8_t>(MessageType::register_node_ack)) {
            message.request_id = make_request_id();
        }
        message.body = error;

        const auto encoded = encode_control(type, message);
        CHECK(encoded.has_value());
        const auto decoded = decode_control(*encoded, type);
        CHECK(decoded.has_value());
        const auto* body = std::get_if<Error>(&decoded->body);
        CHECK(body != nullptr);
        CHECK(body->code == ErrorCode::unavailable);
        CHECK(body->diagnostic == error.diagnostic);
        CHECK(body->component == error.component);
        CHECK(body->node == error.node);
        CHECK(body->execution == error.execution);
    }
}

PLURIBUS_TEST(fabric_control_registry_boundaries) {
    CHECK(is_known_message_type(15) == false);
    CHECK(is_known_message_type(16) == true);
    CHECK(is_known_message_type(37) == true);
    CHECK(is_known_message_type(38) == false);
    CHECK(is_known_message_type(63) == false);
    CHECK(is_known_message_type(64) == true);
    CHECK(is_known_message_type(79) == true);
    CHECK(is_known_message_type(80) == false);
    CHECK(is_known_message_type(0) == false);

    CHECK(is_implemented_message_type(15) == false);
    CHECK(is_implemented_message_type(16) == true);
    CHECK(is_implemented_message_type(19) == true);
    CHECK(is_implemented_message_type(20) == false);
    CHECK(is_implemented_message_type(64) == false);

    CHECK(response_type(16) == std::uint8_t{17});
    CHECK(response_type(17) == std::nullopt);
    CHECK(response_type(18) == std::nullopt);
    CHECK(response_type(20) == std::uint8_t{21});
    CHECK(response_type(22) == std::uint8_t{23});
    CHECK(response_type(79) == std::nullopt);

    // C++ projection matches the numeric registry.
    CHECK(static_cast<std::uint8_t>(MessageType::unspecified) == 0);
    CHECK(static_cast<std::uint8_t>(MessageType::hello) == 1);
    CHECK(static_cast<std::uint8_t>(MessageType::welcome) == 2);
    CHECK(static_cast<std::uint8_t>(MessageType::reject) == 3);
    CHECK(static_cast<std::uint8_t>(MessageType::register_node) == 16);
    CHECK(static_cast<std::uint8_t>(MessageType::register_node_ack) == 17);
    CHECK(static_cast<std::uint8_t>(MessageType::heartbeat) == 18);
    CHECK(static_cast<std::uint8_t>(MessageType::heartbeat_ack) == 19);
    CHECK(static_cast<std::uint8_t>(MessageType::reconcile_decision_ack) == 37);
    CHECK(static_cast<std::uint8_t>(MessageType::log_subscribe) == 64);
    CHECK(static_cast<std::uint8_t>(MessageType::artifact_location_update_ack) == 79);
}

PLURIBUS_TEST(fabric_control_rejects_unknown_bootstrap_and_unimplemented) {
    const auto empty = raw_bytes({});

    const auto unknown = decode_control(empty, 42);
    CHECK(!unknown.has_value());
    CHECK(unknown.error().code == ErrorCode::unsupported_feature);

    const auto bootstrap = decode_control(empty, 1);
    CHECK(!bootstrap.has_value());
    CHECK(bootstrap.error().code == ErrorCode::invalid_argument);

    const auto zero = decode_control(empty, 0);
    CHECK(!zero.has_value());
    CHECK(zero.error().code == ErrorCode::invalid_argument);

    const auto pending = decode_control(empty, 22);
    CHECK(!pending.has_value());
    CHECK(pending.error().code == ErrorCode::unsupported_feature);

    ControlMessage message;
    message.request_id = make_request_id();
    message.body = RegisterNode{make_descriptor(), 1};
    const auto encode_pending = encode_control(22, message);
    CHECK(!encode_pending.has_value());
    CHECK(encode_pending.error().code == ErrorCode::unsupported_feature);
}

PLURIBUS_TEST(fabric_control_payload_type_mismatch) {
    ControlMessage heartbeat;
    heartbeat.body = Heartbeat{make_node_id(), 1, 1, 0, NodeHealth::healthy};
    const auto encoded =
        encode_control(static_cast<std::uint8_t>(MessageType::heartbeat), heartbeat);
    CHECK(encoded.has_value());

    const auto decoded =
        decode_control(*encoded, static_cast<std::uint8_t>(MessageType::register_node));
    CHECK(!decoded.has_value());
    CHECK(decoded.error().code == ErrorCode::invalid_argument);
}

PLURIBUS_TEST(fabric_control_request_id_rules) {
    ControlMessage heartbeat;
    heartbeat.body = Heartbeat{make_node_id(), 1, 1, 0, NodeHealth::healthy};
    const auto encoded =
        encode_control(static_cast<std::uint8_t>(MessageType::heartbeat), heartbeat);
    CHECK(encoded.has_value());

    // Heartbeat must not carry a request_id.
    auto with_id = request_id_field(false);
    with_id.insert(with_id.end(), encoded->begin(), encoded->end());
    const auto decoded =
        decode_control(with_id, static_cast<std::uint8_t>(MessageType::heartbeat));
    CHECK(!decoded.has_value());
    CHECK(decoded.error().code == ErrorCode::invalid_argument);

    ControlMessage encoded_message;
    encoded_message.request_id = make_request_id();
    encoded_message.body = RegisterNode{make_descriptor(), 1};
    const auto register_bytes =
        encode_control(static_cast<std::uint8_t>(MessageType::register_node), encoded_message);
    CHECK(register_bytes.has_value());

    // A request with no request_id is invalid.
    std::vector<std::byte> missing_id(register_bytes->begin() + 18, register_bytes->end());
    const auto missing = decode_control(
        missing_id, static_cast<std::uint8_t>(MessageType::register_node));
    CHECK(!missing.has_value());
    CHECK(missing.error().code == ErrorCode::invalid_argument);

    // A nil request_id is invalid.
    auto nil_id = *register_bytes;
    for (std::size_t index = 2; index < 18; ++index) {
        nil_id[index] = std::byte{0};
    }
    const auto nil = decode_control(nil_id, static_cast<std::uint8_t>(MessageType::register_node));
    CHECK(!nil.has_value());
    CHECK(nil.error().code == ErrorCode::invalid_argument);

    // Encode rejects a nil request_id and a request_id on a notification.
    ControlMessage nil_message = encoded_message;
    nil_message.request_id = RequestId{};
    const auto nil_encode =
        encode_control(static_cast<std::uint8_t>(MessageType::register_node), nil_message);
    CHECK(!nil_encode.has_value());
    CHECK(nil_encode.error().code == ErrorCode::invalid_argument);

    ControlMessage notification = heartbeat;
    notification.request_id = make_request_id();
    const auto notification_encode =
        encode_control(static_cast<std::uint8_t>(MessageType::heartbeat), notification);
    CHECK(!notification_encode.has_value());
    CHECK(notification_encode.error().code == ErrorCode::invalid_argument);
}

PLURIBUS_TEST(fabric_control_rejects_malformed_and_unspecified_error) {
    const auto malformed = decode_control(raw_bytes({0xFF}), 18);
    CHECK(!malformed.has_value());
    CHECK(malformed.error().code == ErrorCode::invalid_argument);

    // Error with the default (unspecified) code on a response type.
    auto error_envelope = request_id_field(false);
    error_envelope.push_back(std::byte{0x12});
    error_envelope.push_back(std::byte{0x00});
    const auto unspecified = decode_control(
        error_envelope, static_cast<std::uint8_t>(MessageType::register_node_ack));
    CHECK(!unspecified.has_value());
    CHECK(unspecified.error().code == ErrorCode::invalid_argument);
}

PLURIBUS_TEST(fabric_control_field_bounds) {
    // Over-long node name.
    ControlMessage node;
    node.request_id = make_request_id();
    RegisterNode register_node{make_descriptor(), 1};
    register_node.descriptor.name = std::string(65, 'a');
    node.body = register_node;
    auto encoded = encode_control(static_cast<std::uint8_t>(MessageType::register_node), node);
    CHECK(!encoded.has_value());
    CHECK(encoded.error().code == ErrorCode::invalid_argument);

    // Too many capabilities.
    RegisterNode many_capabilities{make_descriptor(), 1};
    for (int index = 0; index < 65; ++index) {
        many_capabilities.descriptor.capabilities.insert(
            *CapabilityName::parse("cap" + std::to_string(index)));
    }
    ControlMessage many;
    many.request_id = make_request_id();
    many.body = many_capabilities;
    encoded = encode_control(static_cast<std::uint8_t>(MessageType::register_node), many);
    CHECK(!encoded.has_value());
    CHECK(encoded.error().code == ErrorCode::invalid_argument);

    // Over-long capability (two valid dotted components totalling 65 bytes).
    RegisterNode long_capability{make_descriptor(), 1};
    long_capability.descriptor.capabilities.insert(
        *CapabilityName::parse(std::string(63, 'a') + ".b"));
    ControlMessage long_cap;
    long_cap.request_id = make_request_id();
    long_cap.body = long_capability;
    encoded = encode_control(static_cast<std::uint8_t>(MessageType::register_node), long_cap);
    CHECK(!encoded.has_value());
    CHECK(encoded.error().code == ErrorCode::invalid_argument);

    // Zero incarnation.
    ControlMessage zero_incarnation;
    zero_incarnation.request_id = make_request_id();
    zero_incarnation.body = RegisterNode{make_descriptor(), 0};
    encoded =
        encode_control(static_cast<std::uint8_t>(MessageType::register_node), zero_incarnation);
    CHECK(!encoded.has_value());
    CHECK(encoded.error().code == ErrorCode::invalid_argument);

    // Over-long error diagnostic.
    Error error;
    error.code = ErrorCode::internal;
    error.diagnostic = std::string(1025, 'x');
    ControlMessage error_message;
    error_message.request_id = make_request_id();
    error_message.body = error;
    encoded = encode_control(
        static_cast<std::uint8_t>(MessageType::register_node_ack), error_message);
    CHECK(!encoded.has_value());
    CHECK(encoded.error().code == ErrorCode::invalid_argument);

    // Zero heartbeat interval.
    ControlMessage ack;
    ack.request_id = make_request_id();
    ack.body = RegisterNodeAck{0, false};
    encoded = encode_control(
        static_cast<std::uint8_t>(MessageType::register_node_ack), ack);
    CHECK(!encoded.has_value());
    CHECK(encoded.error().code == ErrorCode::invalid_argument);
}

PLURIBUS_TEST(fabric_control_rejects_oversized_payload) {
    std::vector<std::byte> oversized(kMaxControlPayloadBytes + 1, std::byte{0});
    const auto decoded = decode_control(oversized, 18);
    CHECK(!decoded.has_value());
    CHECK(decoded.error().code == ErrorCode::message_too_large);
}
