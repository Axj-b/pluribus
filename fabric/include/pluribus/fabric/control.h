#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <variant>
#include <vector>

#include "pluribus/core/error.h"
#include "pluribus/core/ids.h"
#include "pluribus/core/node.h"
#include "pluribus/core/resources.h"

namespace pluribus::fabric {

// Numeric message types for the version-1 control protocol. The frame header
// `type` byte is the sole dispatch key: it carries one of these values and the
// Protobuf Envelope has no type field. Values match the generated
// pluribus.wire::MESSAGE_TYPE_* constants (control.proto); static_asserts in
// control_codec.cpp enforce that equality.
//
// Bootstrap Hello/Welcome/Reject stay at frame version 0 and are not wrapped in
// an Envelope.
enum class MessageType : std::uint8_t {
    unspecified = 0,
    hello = 1,
    welcome = 2,
    reject = 3,

    register_node = 16,
    register_node_ack = 17,
    heartbeat = 18,
    heartbeat_ack = 19,
    descriptor_update = 20,
    descriptor_update_ack = 21,
    submit = 22,
    submit_ack = 23,
    get_execution = 24,
    execution_snapshot = 25,
    assign = 26,
    assignment_ack = 27,
    state_event = 28,
    state_event_ack = 29,
    watch_execution = 30,
    execution_event = 31,
    terminate = 32,
    terminate_ack = 33,
    reconcile_request = 34,
    reconcile_report = 35,
    reconcile_decision = 36,
    reconcile_decision_ack = 37,

    log_subscribe = 64,
    log_subscribe_ack = 65,
    service_register = 66,
    service_register_ack = 67,
    service_ready = 68,
    service_ready_ack = 69,
    service_unregister = 70,
    service_unregister_ack = 71,
    service_resolve = 72,
    service_resolve_ack = 73,
    service_watch = 74,
    service_event = 75,
    artifact_location_query = 76,
    artifact_location_ack = 77,
    artifact_location_update = 78,
    artifact_location_update_ack = 79,
};

struct RegisterNode {
    NodeDescriptor descriptor;
    std::uint64_t incarnation = 0;
};

struct RegisterNodeAck {
    std::uint32_t heartbeat_interval_ms = 1000;
    bool reconciliation_required = false;
};

struct Heartbeat {
    NodeId node_id;
    std::uint64_t incarnation = 0;
    std::uint64_t heartbeat_seq = 0;
    Revision descriptor_revision = 0;
    NodeHealth health = NodeHealth::healthy;
};

struct HeartbeatAck {
    std::uint64_t heartbeat_seq = 0;
};

// Exactly one payload. Error lives in the same variant (and the same Protobuf
// oneof) as the bodies so the wire format enforces exactly-one payload.
using ControlBody = std::variant<RegisterNode, RegisterNodeAck, Heartbeat, HeartbeatAck, Error>;

struct ControlMessage {
    std::optional<RequestId> request_id;
    ControlBody body;
};

// Bounded, validated envelope codec. `type` is the frame header type byte.
// Only RegisterNode, RegisterNodeAck, Heartbeat and HeartbeatAck are
// implemented in this revision; other assigned IDs are rejected with
// unsupported_feature until their bodies are defined.
Result<std::vector<std::byte>> encode_control(std::uint8_t type, const ControlMessage& message);

Result<ControlMessage> decode_control(std::span<const std::byte> payload, std::uint8_t type);

// The response/ack that echoes the same request_id for a request type.
// Notifications and already-response types return nullopt. In particular,
// Heartbeat (18) is a notification whose ack (19) correlates by
// heartbeat_seq, not request_id, so response_type(18) is nullopt.
std::optional<std::uint8_t> response_type(std::uint8_t request_type) noexcept;

// Assigned registry ID (16-37, 64-79). Bootstrap and reserved IDs are false.
bool is_known_message_type(std::uint8_t type) noexcept;

// Assigned and has a body in this revision (16-19).
bool is_implemented_message_type(std::uint8_t type) noexcept;

} // namespace pluribus::fabric
