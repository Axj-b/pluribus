#include "pluribus/fabric/control.h"

#include "control.pb.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "pluribus/core/version.h"
#include "pluribus/fabric/frame.h"
#include "pluribus/fabric/message_type.h"

namespace pluribus::fabric {
namespace {

constexpr std::size_t kUuidBytes = 16;
constexpr std::size_t kMaxNameBytes = 64;
constexpr std::size_t kMaxAbiVersionBytes = 64;
constexpr std::size_t kMaxCapabilityBytes = 64;
constexpr std::size_t kMaxCapabilityEntries = 64;
constexpr std::size_t kMaxDeviceEntries = 64;
constexpr std::size_t kMaxDiagnosticBytes = 1024;
constexpr std::size_t kMaxComponentBytes = 64;

// Keep the C++ projection in lock-step with the generated registry. There is
// exactly one numeric assignment per concrete message; the frame type byte is
// the only dispatch key.
static_assert(static_cast<std::uint8_t>(MessageType::unspecified) ==
              wire::MESSAGE_TYPE_UNSPECIFIED);
static_assert(static_cast<std::uint8_t>(MessageType::hello) == wire::MESSAGE_TYPE_HELLO);
static_assert(static_cast<std::uint8_t>(MessageType::welcome) == wire::MESSAGE_TYPE_WELCOME);
static_assert(static_cast<std::uint8_t>(MessageType::reject) == wire::MESSAGE_TYPE_REJECT);
static_assert(static_cast<std::uint8_t>(MessageType::hello) == kBootstrapHelloType);
static_assert(static_cast<std::uint8_t>(MessageType::welcome) == kBootstrapWelcomeType);
static_assert(static_cast<std::uint8_t>(MessageType::reject) == kBootstrapRejectType);
static_assert(static_cast<std::uint8_t>(MessageType::register_node) ==
              wire::MESSAGE_TYPE_REGISTER_NODE);
static_assert(static_cast<std::uint8_t>(MessageType::register_node_ack) ==
              wire::MESSAGE_TYPE_REGISTER_NODE_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::heartbeat) ==
              wire::MESSAGE_TYPE_HEARTBEAT);
static_assert(static_cast<std::uint8_t>(MessageType::heartbeat_ack) ==
              wire::MESSAGE_TYPE_HEARTBEAT_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::descriptor_update) ==
              wire::MESSAGE_TYPE_DESCRIPTOR_UPDATE);
static_assert(static_cast<std::uint8_t>(MessageType::descriptor_update_ack) ==
              wire::MESSAGE_TYPE_DESCRIPTOR_UPDATE_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::submit) == wire::MESSAGE_TYPE_SUBMIT);
static_assert(static_cast<std::uint8_t>(MessageType::submit_ack) ==
              wire::MESSAGE_TYPE_SUBMIT_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::get_execution) ==
              wire::MESSAGE_TYPE_GET_EXECUTION);
static_assert(static_cast<std::uint8_t>(MessageType::execution_snapshot) ==
              wire::MESSAGE_TYPE_EXECUTION_SNAPSHOT);
static_assert(static_cast<std::uint8_t>(MessageType::assign) == wire::MESSAGE_TYPE_ASSIGN);
static_assert(static_cast<std::uint8_t>(MessageType::assignment_ack) ==
              wire::MESSAGE_TYPE_ASSIGNMENT_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::state_event) ==
              wire::MESSAGE_TYPE_STATE_EVENT);
static_assert(static_cast<std::uint8_t>(MessageType::state_event_ack) ==
              wire::MESSAGE_TYPE_STATE_EVENT_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::watch_execution) ==
              wire::MESSAGE_TYPE_WATCH_EXECUTION);
static_assert(static_cast<std::uint8_t>(MessageType::execution_event) ==
              wire::MESSAGE_TYPE_EXECUTION_EVENT);
static_assert(static_cast<std::uint8_t>(MessageType::terminate) ==
              wire::MESSAGE_TYPE_TERMINATE);
static_assert(static_cast<std::uint8_t>(MessageType::terminate_ack) ==
              wire::MESSAGE_TYPE_TERMINATE_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::reconcile_request) ==
              wire::MESSAGE_TYPE_RECONCILE_REQUEST);
static_assert(static_cast<std::uint8_t>(MessageType::reconcile_report) ==
              wire::MESSAGE_TYPE_RECONCILE_REPORT);
static_assert(static_cast<std::uint8_t>(MessageType::reconcile_decision) ==
              wire::MESSAGE_TYPE_RECONCILE_DECISION);
static_assert(static_cast<std::uint8_t>(MessageType::reconcile_decision_ack) ==
              wire::MESSAGE_TYPE_RECONCILE_DECISION_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::log_subscribe) ==
              wire::MESSAGE_TYPE_LOG_SUBSCRIBE);
static_assert(static_cast<std::uint8_t>(MessageType::log_subscribe_ack) ==
              wire::MESSAGE_TYPE_LOG_SUBSCRIBE_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::service_register) ==
              wire::MESSAGE_TYPE_SERVICE_REGISTER);
static_assert(static_cast<std::uint8_t>(MessageType::service_register_ack) ==
              wire::MESSAGE_TYPE_SERVICE_REGISTER_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::service_ready) ==
              wire::MESSAGE_TYPE_SERVICE_READY);
static_assert(static_cast<std::uint8_t>(MessageType::service_ready_ack) ==
              wire::MESSAGE_TYPE_SERVICE_READY_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::service_unregister) ==
              wire::MESSAGE_TYPE_SERVICE_UNREGISTER);
static_assert(static_cast<std::uint8_t>(MessageType::service_unregister_ack) ==
              wire::MESSAGE_TYPE_SERVICE_UNREGISTER_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::service_resolve) ==
              wire::MESSAGE_TYPE_SERVICE_RESOLVE);
static_assert(static_cast<std::uint8_t>(MessageType::service_resolve_ack) ==
              wire::MESSAGE_TYPE_SERVICE_RESOLVE_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::service_watch) ==
              wire::MESSAGE_TYPE_SERVICE_WATCH);
static_assert(static_cast<std::uint8_t>(MessageType::service_event) ==
              wire::MESSAGE_TYPE_SERVICE_EVENT);
static_assert(static_cast<std::uint8_t>(MessageType::artifact_location_query) ==
              wire::MESSAGE_TYPE_ARTIFACT_LOCATION_QUERY);
static_assert(static_cast<std::uint8_t>(MessageType::artifact_location_ack) ==
              wire::MESSAGE_TYPE_ARTIFACT_LOCATION_ACK);
static_assert(static_cast<std::uint8_t>(MessageType::artifact_location_update) ==
              wire::MESSAGE_TYPE_ARTIFACT_LOCATION_UPDATE);
static_assert(static_cast<std::uint8_t>(MessageType::artifact_location_update_ack) ==
              wire::MESSAGE_TYPE_ARTIFACT_LOCATION_UPDATE_ACK);

Error invalid(std::string diagnostic) {
    return make_error(ErrorCode::invalid_argument, std::move(diagnostic));
}

Error unsupported_feature(std::string diagnostic) {
    return make_error(ErrorCode::unsupported_feature, std::move(diagnostic));
}

Error message_too_large(std::string diagnostic) {
    return make_error(ErrorCode::message_too_large, std::move(diagnostic));
}

bool serialized(const google::protobuf::MessageLite& message, std::vector<std::byte>& output) {
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

Result<std::vector<std::byte>> finish(const google::protobuf::MessageLite& message) {
    std::vector<std::byte> output;
    if (!serialized(message, output)) {
        return std::unexpected(make_error(ErrorCode::internal, "protobuf serialization failed"));
    }
    return output;
}

std::string to_wire_uuid(const Uuid& uuid) {
    const auto& bytes = uuid.bytes();
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

bool nil_bytes(std::string_view raw) noexcept {
    for (const char byte : raw) {
        if (byte != '\0') {
            return false;
        }
    }
    return true;
}

// Strict UTF-8 validation over the raw byte sequence: rejects overlong forms,
// surrogate code points and values above U+10FFFF. Protobuf `string` fields
// must be valid UTF-8; serialization and parsing both depend on it.
bool is_valid_utf8(std::string_view text) noexcept {
    std::size_t index = 0;
    while (index < text.size()) {
        const auto lead = static_cast<unsigned char>(text[index]);
        if (lead < 0x80) {
            ++index;
            continue;
        }

        std::size_t continuation = 0;
        unsigned int codepoint = 0;
        unsigned int minimum = 0;
        if ((lead & 0xE0) == 0xC0) {
            continuation = 1;
            codepoint = lead & 0x1Fu;
            minimum = 0x80;
        } else if ((lead & 0xF0) == 0xE0) {
            continuation = 2;
            codepoint = lead & 0x0Fu;
            minimum = 0x800;
        } else if ((lead & 0xF8) == 0xF0) {
            continuation = 3;
            codepoint = lead & 0x07u;
            minimum = 0x10000;
        } else {
            return false;
        }

        if (text.size() - index <= continuation) {
            return false;
        }
        for (std::size_t offset = 1; offset <= continuation; ++offset) {
            const auto next = static_cast<unsigned char>(text[index + offset]);
            if ((next & 0xC0) != 0x80) {
                return false;
            }
            codepoint = (codepoint << 6) | (next & 0x3Fu);
        }
        if (codepoint < minimum || codepoint > 0x10FFFF ||
            (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
            return false;
        }
        index += continuation + 1;
    }
    return true;
}

bool read_uuid(std::string_view raw, Uuid& uuid) {
    if (raw.size() != kUuidBytes || nil_bytes(raw)) {
        return false;
    }
    Uuid::Bytes bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::uint8_t>(static_cast<unsigned char>(raw[index]));
    }
    uuid = Uuid(bytes);
    return true;
}

bool requires_request_id(std::uint8_t type) noexcept {
    // Request and response messages echo the correlation ID. Notifications and
    // acks do not carry one (heartbeat uses heartbeat_seq instead).
    return type == static_cast<std::uint8_t>(MessageType::register_node) ||
           type == static_cast<std::uint8_t>(MessageType::register_node_ack);
}

bool accepts_error_payload(std::uint8_t type) noexcept {
    // Error is only legal on response/ack messages. In this revision that means
    // RegisterNodeAck and HeartbeatAck.
    return type == static_cast<std::uint8_t>(MessageType::register_node_ack) ||
           type == static_cast<std::uint8_t>(MessageType::heartbeat_ack);
}

wire::ErrorCode to_wire_error_code(ErrorCode code) {
    switch (code) {
    case ErrorCode::invalid_argument:
        return wire::ERROR_CODE_INVALID_ARGUMENT;
    case ErrorCode::message_too_large:
        return wire::ERROR_CODE_MESSAGE_TOO_LARGE;
    case ErrorCode::unsupported_version:
        return wire::ERROR_CODE_UNSUPPORTED_VERSION;
    case ErrorCode::unsupported_feature:
        return wire::ERROR_CODE_UNSUPPORTED_FEATURE;
    case ErrorCode::unauthorized:
        return wire::ERROR_CODE_UNAUTHORIZED;
    case ErrorCode::not_found:
        return wire::ERROR_CODE_NOT_FOUND;
    case ErrorCode::ambiguous:
        return wire::ERROR_CODE_AMBIGUOUS;
    case ErrorCode::conflict:
        return wire::ERROR_CODE_CONFLICT;
    case ErrorCode::unavailable:
        return wire::ERROR_CODE_UNAVAILABLE;
    case ErrorCode::timeout:
        return wire::ERROR_CODE_TIMEOUT;
    case ErrorCode::resource_exhausted:
        return wire::ERROR_CODE_RESOURCE_EXHAUSTED;
    case ErrorCode::unschedulable:
        return wire::ERROR_CODE_UNSCHEDULABLE;
    case ErrorCode::sequence_gap:
        return wire::ERROR_CODE_SEQUENCE_GAP;
    case ErrorCode::resync_required:
        return wire::ERROR_CODE_RESYNC_REQUIRED;
    case ErrorCode::artifact_missing:
        return wire::ERROR_CODE_ARTIFACT_MISSING;
    case ErrorCode::integrity_error:
        return wire::ERROR_CODE_INTEGRITY_ERROR;
    case ErrorCode::launch_failed:
        return wire::ERROR_CODE_LAUNCH_FAILED;
    case ErrorCode::internal:
        return wire::ERROR_CODE_INTERNAL;
    }
    return wire::ERROR_CODE_INTERNAL;
}

std::optional<ErrorCode> from_wire_error_code(wire::ErrorCode code) {
    switch (code) {
    case wire::ERROR_CODE_INVALID_ARGUMENT:
        return ErrorCode::invalid_argument;
    case wire::ERROR_CODE_MESSAGE_TOO_LARGE:
        return ErrorCode::message_too_large;
    case wire::ERROR_CODE_UNSUPPORTED_VERSION:
        return ErrorCode::unsupported_version;
    case wire::ERROR_CODE_UNSUPPORTED_FEATURE:
        return ErrorCode::unsupported_feature;
    case wire::ERROR_CODE_UNAUTHORIZED:
        return ErrorCode::unauthorized;
    case wire::ERROR_CODE_NOT_FOUND:
        return ErrorCode::not_found;
    case wire::ERROR_CODE_AMBIGUOUS:
        return ErrorCode::ambiguous;
    case wire::ERROR_CODE_CONFLICT:
        return ErrorCode::conflict;
    case wire::ERROR_CODE_UNAVAILABLE:
        return ErrorCode::unavailable;
    case wire::ERROR_CODE_TIMEOUT:
        return ErrorCode::timeout;
    case wire::ERROR_CODE_RESOURCE_EXHAUSTED:
        return ErrorCode::resource_exhausted;
    case wire::ERROR_CODE_UNSCHEDULABLE:
        return ErrorCode::unschedulable;
    case wire::ERROR_CODE_SEQUENCE_GAP:
        return ErrorCode::sequence_gap;
    case wire::ERROR_CODE_RESYNC_REQUIRED:
        return ErrorCode::resync_required;
    case wire::ERROR_CODE_ARTIFACT_MISSING:
        return ErrorCode::artifact_missing;
    case wire::ERROR_CODE_INTEGRITY_ERROR:
        return ErrorCode::integrity_error;
    case wire::ERROR_CODE_LAUNCH_FAILED:
        return ErrorCode::launch_failed;
    case wire::ERROR_CODE_INTERNAL:
        return ErrorCode::internal;
    case wire::ERROR_CODE_UNSPECIFIED:
        return std::nullopt;
    default:
        return std::nullopt;
    }
}

std::optional<wire::CpuArchitecture> to_wire_architecture(CpuArchitecture architecture) {
    switch (architecture) {
    case CpuArchitecture::x86_64:
        return wire::CPU_ARCHITECTURE_X86_64;
    case CpuArchitecture::arm64:
        return wire::CPU_ARCHITECTURE_ARM64;
    }
    return std::nullopt;
}

std::optional<CpuArchitecture> from_wire_architecture(wire::CpuArchitecture architecture) {
    switch (architecture) {
    case wire::CPU_ARCHITECTURE_X86_64:
        return CpuArchitecture::x86_64;
    case wire::CPU_ARCHITECTURE_ARM64:
        return CpuArchitecture::arm64;
    case wire::CPU_ARCHITECTURE_UNSPECIFIED:
        return std::nullopt;
    default:
        return std::nullopt;
    }
}

std::optional<wire::NodeHealth> to_wire_health(NodeHealth health) {
    switch (health) {
    case NodeHealth::healthy:
        return wire::NODE_HEALTH_HEALTHY;
    case NodeHealth::degraded:
        return wire::NODE_HEALTH_DEGRADED;
    case NodeHealth::unhealthy:
        return wire::NODE_HEALTH_UNHEALTHY;
    }
    return std::nullopt;
}

std::optional<NodeHealth> from_wire_health(wire::NodeHealth health) {
    switch (health) {
    case wire::NODE_HEALTH_HEALTHY:
        return NodeHealth::healthy;
    case wire::NODE_HEALTH_DEGRADED:
        return NodeHealth::degraded;
    case wire::NODE_HEALTH_UNHEALTHY:
        return NodeHealth::unhealthy;
    case wire::NODE_HEALTH_UNSPECIFIED:
        return std::nullopt;
    default:
        return std::nullopt;
    }
}

std::optional<Error> validate_descriptor(const NodeDescriptor& descriptor) {
    if (descriptor.id.is_nil()) {
        return invalid("node descriptor id is nil");
    }
    if (descriptor.name.size() > kMaxNameBytes) {
        return invalid("node descriptor name is too long");
    }
    if (!is_valid_utf8(descriptor.name)) {
        return invalid("node descriptor name is not valid UTF-8");
    }
    if (descriptor.abi_version.value().size() > kMaxAbiVersionBytes) {
        return invalid("node descriptor abi version is too long");
    }
    if (!to_wire_architecture(descriptor.architecture)) {
        return invalid("node descriptor architecture is invalid");
    }
    if (descriptor.capabilities.size() > kMaxCapabilityEntries) {
        return invalid("node descriptor has too many capabilities");
    }
    for (const auto& capability : descriptor.capabilities) {
        if (capability.empty() || capability.value().size() > kMaxCapabilityBytes) {
            return invalid("node descriptor capability is empty or too long");
        }
    }
    if (descriptor.devices.size() > kMaxDeviceEntries) {
        return invalid("node descriptor has too many devices");
    }
    for (const auto& device : descriptor.devices) {
        if (device.empty() || device.value().size() > kMaxCapabilityBytes) {
            return invalid("node descriptor device is empty or too long");
        }
    }
    return std::nullopt;
}

void write_error(const Error& error, wire::Error& out) {
    out.set_code(to_wire_error_code(error.code));
    out.set_diagnostic(error.diagnostic);
    out.set_component(error.component);
    if (error.node) {
        out.set_node_id(to_wire_uuid(error.node->value()));
    }
    if (error.execution) {
        out.set_execution_id(to_wire_uuid(error.execution->value()));
    }
}

std::optional<Error> read_error(const wire::Error& in, Error& error) {
    const auto code = from_wire_error_code(in.code());
    if (!code) {
        return invalid("error code is unspecified or unknown");
    }
    error.code = *code;
    if (in.diagnostic().size() > kMaxDiagnosticBytes) {
        return invalid("error diagnostic is too long");
    }
    if (!is_valid_utf8(in.diagnostic())) {
        return invalid("error diagnostic is not valid UTF-8");
    }
    error.diagnostic = in.diagnostic();
    if (in.component().size() > kMaxComponentBytes) {
        return invalid("error component is too long");
    }
    if (!is_valid_utf8(in.component())) {
        return invalid("error component is not valid UTF-8");
    }
    error.component = in.component();
    if (!in.node_id().empty()) {
        Uuid node;
        if (!read_uuid(in.node_id(), node)) {
            return invalid("error node id is not a 16-byte non-nil UUID");
        }
        error.node = NodeId(node);
    }
    if (!in.execution_id().empty()) {
        Uuid execution;
        if (!read_uuid(in.execution_id(), execution)) {
            return invalid("error execution id is not a 16-byte non-nil UUID");
        }
        error.execution = ExecutionId(execution);
    }
    return std::nullopt;
}

std::optional<Error> validate_error(const Error& error) {
    if (error.diagnostic.size() > kMaxDiagnosticBytes) {
        return invalid("error diagnostic is too long");
    }
    if (!is_valid_utf8(error.diagnostic)) {
        return invalid("error diagnostic is not valid UTF-8");
    }
    if (error.component.size() > kMaxComponentBytes) {
        return invalid("error component is too long");
    }
    if (!is_valid_utf8(error.component)) {
        return invalid("error component is not valid UTF-8");
    }
    if (error.node && error.node->is_nil()) {
        return invalid("error node id is nil");
    }
    if (error.execution && error.execution->is_nil()) {
        return invalid("error execution id is nil");
    }
    return std::nullopt;
}

void write_descriptor(const NodeDescriptor& descriptor, wire::NodeDescriptor& out) {
    out.set_id(to_wire_uuid(descriptor.id.value()));
    out.set_name(descriptor.name);
    out.set_protocol_major(descriptor.protocol_major);
    out.set_abi_version(descriptor.abi_version.to_string());
    out.set_architecture(*to_wire_architecture(descriptor.architecture));
    out.set_logical_cpu_count(descriptor.logical_cpu_count);
    out.set_total_memory_bytes(descriptor.total_memory_bytes);
    for (const auto& capability : descriptor.capabilities) {
        out.add_capabilities(capability.to_string());
    }
    for (const auto& device : descriptor.devices) {
        out.add_devices(device.to_string());
    }
}

std::optional<Error> read_descriptor(
    const wire::NodeDescriptor& in,
    NodeDescriptor& descriptor) {
    Uuid id;
    if (!read_uuid(in.id(), id)) {
        return invalid("node descriptor id is not a 16-byte non-nil UUID");
    }
    descriptor.id = NodeId(id);
    if (in.name().size() > kMaxNameBytes) {
        return invalid("node descriptor name is too long");
    }
    descriptor.name = in.name();
    descriptor.protocol_major = in.protocol_major();
    if (in.abi_version().size() > kMaxAbiVersionBytes) {
        return invalid("node descriptor abi version is too long");
    }
    if (!in.abi_version().empty()) {
        const auto abi = AbiVersion::parse(in.abi_version());
        if (!abi) {
            return invalid("node descriptor abi version is invalid");
        }
        descriptor.abi_version = *abi;
    }
    const auto architecture = from_wire_architecture(in.architecture());
    if (!architecture) {
        return invalid("node descriptor architecture is unspecified or unknown");
    }
    descriptor.architecture = *architecture;
    descriptor.logical_cpu_count = in.logical_cpu_count();
    descriptor.total_memory_bytes = in.total_memory_bytes();
    if (in.capabilities_size() > static_cast<int>(kMaxCapabilityEntries)) {
        return invalid("node descriptor has too many capabilities");
    }
    for (const auto& capability : in.capabilities()) {
        if (capability.size() > kMaxCapabilityBytes) {
            return invalid("node descriptor capability is too long");
        }
        const auto parsed = CapabilityName::parse(capability);
        if (!parsed) {
            return invalid("node descriptor capability is invalid");
        }
        descriptor.capabilities.insert(*parsed);
    }
    if (in.devices_size() > static_cast<int>(kMaxDeviceEntries)) {
        return invalid("node descriptor has too many devices");
    }
    for (const auto& device : in.devices()) {
        if (device.size() > kMaxCapabilityBytes) {
            return invalid("node descriptor device is too long");
        }
        const auto parsed = CapabilityName::parse(device);
        if (!parsed) {
            return invalid("node descriptor device is invalid");
        }
        descriptor.devices.insert(*parsed);
    }
    return std::nullopt;
}

bool payload_matches_type(const wire::Envelope& envelope, std::uint8_t type) noexcept {
    const bool error_allowed =
        envelope.payload_case() == wire::Envelope::kError && accepts_error_payload(type);
    switch (type) {
    case static_cast<std::uint8_t>(MessageType::register_node):
        return envelope.payload_case() == wire::Envelope::kRegisterNode;
    case static_cast<std::uint8_t>(MessageType::register_node_ack):
        return envelope.payload_case() == wire::Envelope::kRegisterNodeAck || error_allowed;
    case static_cast<std::uint8_t>(MessageType::heartbeat):
        return envelope.payload_case() == wire::Envelope::kHeartbeat;
    case static_cast<std::uint8_t>(MessageType::heartbeat_ack):
        return envelope.payload_case() == wire::Envelope::kHeartbeatAck || error_allowed;
    default:
        return false;
    }
}

} // namespace

std::optional<std::uint8_t> response_type(std::uint8_t request_type) noexcept {
    switch (request_type) {
    case static_cast<std::uint8_t>(MessageType::register_node):
        return static_cast<std::uint8_t>(MessageType::register_node_ack);
    case static_cast<std::uint8_t>(MessageType::descriptor_update):
        return static_cast<std::uint8_t>(MessageType::descriptor_update_ack);
    case static_cast<std::uint8_t>(MessageType::submit):
        return static_cast<std::uint8_t>(MessageType::submit_ack);
    case static_cast<std::uint8_t>(MessageType::get_execution):
        return static_cast<std::uint8_t>(MessageType::execution_snapshot);
    case static_cast<std::uint8_t>(MessageType::assign):
        return static_cast<std::uint8_t>(MessageType::assignment_ack);
    case static_cast<std::uint8_t>(MessageType::state_event):
        return static_cast<std::uint8_t>(MessageType::state_event_ack);
    case static_cast<std::uint8_t>(MessageType::terminate):
        return static_cast<std::uint8_t>(MessageType::terminate_ack);
    case static_cast<std::uint8_t>(MessageType::reconcile_request):
        return static_cast<std::uint8_t>(MessageType::reconcile_report);
    case static_cast<std::uint8_t>(MessageType::reconcile_decision):
        return static_cast<std::uint8_t>(MessageType::reconcile_decision_ack);
    case static_cast<std::uint8_t>(MessageType::log_subscribe):
        return static_cast<std::uint8_t>(MessageType::log_subscribe_ack);
    case static_cast<std::uint8_t>(MessageType::service_register):
        return static_cast<std::uint8_t>(MessageType::service_register_ack);
    case static_cast<std::uint8_t>(MessageType::service_ready):
        return static_cast<std::uint8_t>(MessageType::service_ready_ack);
    case static_cast<std::uint8_t>(MessageType::service_unregister):
        return static_cast<std::uint8_t>(MessageType::service_unregister_ack);
    case static_cast<std::uint8_t>(MessageType::service_resolve):
        return static_cast<std::uint8_t>(MessageType::service_resolve_ack);
    case static_cast<std::uint8_t>(MessageType::artifact_location_query):
        return static_cast<std::uint8_t>(MessageType::artifact_location_ack);
    case static_cast<std::uint8_t>(MessageType::artifact_location_update):
        return static_cast<std::uint8_t>(MessageType::artifact_location_update_ack);
    default:
        return std::nullopt;
    }
}

bool is_known_message_type(std::uint8_t type) noexcept {
    return type >= 16 && wire::MessageType_IsValid(static_cast<int>(type));
}

bool is_implemented_message_type(std::uint8_t type) noexcept {
    return type >= static_cast<std::uint8_t>(MessageType::register_node) &&
           type <= static_cast<std::uint8_t>(MessageType::heartbeat_ack);
}

Result<std::vector<std::byte>> encode_control(
    std::uint8_t type,
    const ControlMessage& message) {
    if (type <= static_cast<std::uint8_t>(MessageType::reject)) {
        return std::unexpected(invalid("bootstrap type not valid in a control envelope"));
    }
    if (!is_known_message_type(type)) {
        return std::unexpected(unsupported_feature("unknown message type"));
    }
    if (!is_implemented_message_type(type)) {
        return std::unexpected(
            unsupported_feature("message type is assigned but not yet implemented"));
    }
    if (requires_request_id(type)) {
        if (!message.request_id) {
            return std::unexpected(invalid("request_id is required for this message type"));
        }
        if (message.request_id->is_nil()) {
            return std::unexpected(invalid("request_id is nil"));
        }
    } else if (message.request_id) {
        return std::unexpected(invalid("request_id must be absent for this message type"));
    }

    wire::Envelope envelope;
    if (message.request_id) {
        envelope.set_request_id(to_wire_uuid(message.request_id->value()));
    }

    switch (type) {
    case static_cast<std::uint8_t>(MessageType::register_node): {
        const auto* body = std::get_if<RegisterNode>(&message.body);
        if (body == nullptr) {
            return std::unexpected(invalid("control body does not match message type"));
        }
        if (body->incarnation == 0) {
            return std::unexpected(invalid("register_node incarnation must be at least 1"));
        }
        if (const auto error = validate_descriptor(body->descriptor)) {
            return std::unexpected(*error);
        }
        auto* out = envelope.mutable_register_node();
        write_descriptor(body->descriptor, *out->mutable_node_descriptor());
        out->set_incarnation(body->incarnation);
        break;
    }
    case static_cast<std::uint8_t>(MessageType::register_node_ack): {
        if (const auto* body = std::get_if<RegisterNodeAck>(&message.body)) {
            if (body->heartbeat_interval_ms == 0) {
                return std::unexpected(
                    invalid("register_node_ack heartbeat interval must be at least 1"));
            }
            auto* out = envelope.mutable_register_node_ack();
            out->set_heartbeat_interval_ms(body->heartbeat_interval_ms);
            out->set_reconciliation_required(body->reconciliation_required);
        } else if (const auto* error = std::get_if<Error>(&message.body)) {
            if (const auto validated = validate_error(*error)) {
                return std::unexpected(*validated);
            }
            write_error(*error, *envelope.mutable_error());
        } else {
            return std::unexpected(invalid("control body does not match message type"));
        }
        break;
    }
    case static_cast<std::uint8_t>(MessageType::heartbeat): {
        const auto* body = std::get_if<Heartbeat>(&message.body);
        if (body == nullptr) {
            return std::unexpected(invalid("control body does not match message type"));
        }
        if (body->node_id.is_nil()) {
            return std::unexpected(invalid("heartbeat node id is nil"));
        }
        if (body->incarnation == 0) {
            return std::unexpected(invalid("heartbeat incarnation must be at least 1"));
        }
        const auto health = to_wire_health(body->health);
        if (!health) {
            return std::unexpected(invalid("heartbeat health is invalid"));
        }
        auto* out = envelope.mutable_heartbeat();
        out->set_node_id(to_wire_uuid(body->node_id.value()));
        out->set_incarnation(body->incarnation);
        out->set_heartbeat_seq(body->heartbeat_seq);
        out->set_descriptor_revision(body->descriptor_revision);
        out->set_health(*health);
        break;
    }
    case static_cast<std::uint8_t>(MessageType::heartbeat_ack): {
        if (const auto* body = std::get_if<HeartbeatAck>(&message.body)) {
            envelope.mutable_heartbeat_ack()->set_heartbeat_seq(body->heartbeat_seq);
        } else if (const auto* error = std::get_if<Error>(&message.body)) {
            if (const auto validated = validate_error(*error)) {
                return std::unexpected(*validated);
            }
            write_error(*error, *envelope.mutable_error());
        } else {
            return std::unexpected(invalid("control body does not match message type"));
        }
        break;
    }
    default:
        return std::unexpected(
            unsupported_feature("message type is assigned but not yet implemented"));
    }

    return finish(envelope);
}

Result<ControlMessage> decode_control(std::span<const std::byte> payload, std::uint8_t type) {
    // Bootstrap (Hello/Welcome/Reject) and the invalid type 0 are illegal here.
    // Check this before the registry lookup so those cases report
    // invalid_argument rather than unsupported_feature.
    if (type <= static_cast<std::uint8_t>(MessageType::reject)) {
        return std::unexpected(invalid("bootstrap type not valid in a control envelope"));
    }
    if (!is_known_message_type(type)) {
        return std::unexpected(unsupported_feature("unknown message type"));
    }
    if (!is_implemented_message_type(type)) {
        return std::unexpected(
            unsupported_feature("message type is assigned but not yet implemented"));
    }
    if (payload.size() > static_cast<std::size_t>(kMaxFrameLength - kHeaderBytes)) {
        return std::unexpected(message_too_large("control payload exceeds the frame limit"));
    }

    wire::Envelope envelope;
    if (!envelope.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        return std::unexpected(invalid("invalid control envelope"));
    }
    if (!payload_matches_type(envelope, type)) {
        return std::unexpected(invalid("control envelope payload does not match message type"));
    }

    ControlMessage message;
    if (requires_request_id(type)) {
        if (envelope.request_id().empty()) {
            return std::unexpected(invalid("request_id is required for this message type"));
        }
        Uuid request_id;
        if (!read_uuid(envelope.request_id(), request_id)) {
            return std::unexpected(invalid("request_id is not a 16-byte non-nil UUID"));
        }
        message.request_id = RequestId(request_id);
    } else if (!envelope.request_id().empty()) {
        return std::unexpected(invalid("request_id must be absent for this message type"));
    }

    switch (type) {
    case static_cast<std::uint8_t>(MessageType::register_node): {
        const auto& in = envelope.register_node();
        if (!in.has_node_descriptor()) {
            return std::unexpected(invalid("register_node descriptor is missing"));
        }
        RegisterNode body;
        if (const auto error = read_descriptor(in.node_descriptor(), body.descriptor)) {
            return std::unexpected(*error);
        }
        if (in.incarnation() == 0) {
            return std::unexpected(invalid("register_node incarnation must be at least 1"));
        }
        body.incarnation = in.incarnation();
        message.body = std::move(body);
        break;
    }
    case static_cast<std::uint8_t>(MessageType::register_node_ack): {
        if (envelope.payload_case() == wire::Envelope::kError) {
            Error error;
            if (const auto validated = read_error(envelope.error(), error)) {
                return std::unexpected(*validated);
            }
            message.body = std::move(error);
        } else {
            const auto& in = envelope.register_node_ack();
            if (in.heartbeat_interval_ms() == 0) {
                return std::unexpected(
                    invalid("register_node_ack heartbeat interval must be at least 1"));
            }
            RegisterNodeAck body;
            body.heartbeat_interval_ms = in.heartbeat_interval_ms();
            body.reconciliation_required = in.reconciliation_required();
            message.body = body;
        }
        break;
    }
    case static_cast<std::uint8_t>(MessageType::heartbeat): {
        const auto& in = envelope.heartbeat();
        Uuid node;
        if (!read_uuid(in.node_id(), node)) {
            return std::unexpected(invalid("heartbeat node id is not a 16-byte non-nil UUID"));
        }
        if (in.incarnation() == 0) {
            return std::unexpected(invalid("heartbeat incarnation must be at least 1"));
        }
        const auto health = from_wire_health(in.health());
        if (!health) {
            return std::unexpected(invalid("heartbeat health is unspecified or unknown"));
        }
        Heartbeat body;
        body.node_id = NodeId(node);
        body.incarnation = in.incarnation();
        body.heartbeat_seq = in.heartbeat_seq();
        body.descriptor_revision = in.descriptor_revision();
        body.health = *health;
        message.body = body;
        break;
    }
    case static_cast<std::uint8_t>(MessageType::heartbeat_ack): {
        if (envelope.payload_case() == wire::Envelope::kError) {
            Error error;
            if (const auto validated = read_error(envelope.error(), error)) {
                return std::unexpected(*validated);
            }
            message.body = std::move(error);
        } else {
            HeartbeatAck body;
            body.heartbeat_seq = envelope.heartbeat_ack().heartbeat_seq();
            message.body = body;
        }
        break;
    }
    default:
        return std::unexpected(
            unsupported_feature("message type is assigned but not yet implemented"));
    }

    return message;
}

} // namespace pluribus::fabric
