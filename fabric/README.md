# Using the fabric control codec

The control codec is the payload layer for version-1 control traffic. It turns plain C++ structs
into the Protobuf `Envelope` bytes that ride inside a control frame. Bootstrap (Hello/Welcome/Reject)
is separate and does **not** use this layer.

> This is a developer guide, not the canonical spec. The contracts live in
> [`docs/spec/05-wire-protocol.md`](../docs/spec/05-wire-protocol.md) and
> [`docs/spec/02-node-master-protocol.md`](../docs/spec/02-node-master-protocol.md); the numeric
> registry is [`proto/control.proto`](proto/control.proto).

## The two things you must know

1. **The frame header `type` byte is the dispatch key.** There is no type field inside the payload.
   Every concrete message has one `MessageType` value, and `encode_control`/`decode_control` take that
   value explicitly. `Envelope` carries only `request_id` plus a `oneof` body.

2. **`request_id` correlation is three-way:**
   - **request** (`RegisterNode`, 16) — `request_id` required.
   - **response** (`RegisterNodeAck`, 17) — `request_id` required, echoes the request.
   - **notification / seq-ack** (`Heartbeat` 18, `HeartbeatAck` 19) — `request_id` forbidden; they
     correlate via `heartbeat_seq` instead.

```cpp
#include <pluribus/fabric/control.h>
#include <pluribus/fabric/frame.h>
#include <pluribus/core/ids.h>      // NodeId, RequestId
#include <pluribus/core/node.h>     // NodeDescriptor, NodeHealth
#include <pluribus/core/names.h>    // CapabilityName
#include <pluribus/core/resources.h>// AbiVersion, CpuArchitecture
```

## Encoding a message

1. Build the body struct (plain core types, no Protobuf types leak into your code).
2. Wrap it in a `ControlMessage` (set `request_id` only when the type requires it).
3. `encode_control(type, message)` returns the payload bytes or an `Error`.
4. Put those bytes in a `Frame` with `version = 1` and `type = MessageType`, then `encode_frame`.

```cpp
using namespace pluribus;
using namespace pluribus::fabric;

NodeDescriptor descriptor;
descriptor.id        = *NodeId::parse("aaaaaaaa-bbbb-4ccc-8ddd-eeeeeeeeeeee");
descriptor.name      = "node-alpha";
descriptor.protocol_major = 1;
descriptor.abi_version    = *AbiVersion::parse("pluribus-v1");
descriptor.architecture   = CpuArchitecture::x86_64;
descriptor.logical_cpu_count = 8;
descriptor.total_memory_bytes = 16ull * 1024 * 1024 * 1024;
descriptor.capabilities.insert(*CapabilityName::parse("gpu.cuda"));

ControlMessage message;
message.request_id = *RequestId::parse("11111111-2222-4333-8444-555555555555");
message.body       = RegisterNode{descriptor, /*incarnation=*/7};

const auto type    = static_cast<std::uint8_t>(MessageType::register_node);
auto payload = encode_control(type, message);
if (!payload) {
    // payload.error() is an Error: inspect .code and .diagnostic.
    return;
}

Frame frame;
frame.header.version = kProtocolVersion;   // 1
frame.header.type    = type;
frame.payload        = std::move(*payload);

auto bytes = encode_frame(frame);           // ready to send over a Stream/session
```

## Decoding a message

```cpp
auto decoded = decode_frame(received_bytes);        // validates version/flags/size
if (!decoded) { /* framing error: close the connection */ }

const auto& frame = *decoded;
if (frame.header.version != kProtocolVersion) {
    // bootstrap or unknown version — handle separately
}

auto message = decode_control(frame.payload, frame.header.type);
if (!message) {
    // message.error().code tells you why (invalid_argument, unsupported_feature, ...)
    return;
}

// Dispatch on the body variant. The type already told you what to expect, but
// the variant is the safe way to read it.
if (auto* body = std::get_if<RegisterNode>(&message->body)) {
    // body->descriptor, body->incarnation, message->request_id
}
```

`decode_control` rejects, in order: bootstrap/invalid types, unknown types, assigned-but-unimplemented
types (20–79 today), oversized payloads, malformed protobuf, a body that does not match the type,
wrong `request_id` presence, and out-of-bounds/invalid field values. Unknown optional fields are
tolerated on decode but are not preserved if you re-encode.

## The registry at a glance

| Type | Value | Direction | Correlation |
|---|---:|---|---|
| `register_node` | 16 | node → master | `request_id` |
| `register_node_ack` | 17 | master → node | `request_id` (echo) |
| `heartbeat` | 18 | node → master | `heartbeat_seq` |
| `heartbeat_ack` | 19 | master → node | `heartbeat_seq` (echo) |

Helpful helpers: `is_known_message_type`, `is_implemented_message_type`, and
`response_type` (16 → 17; returns `std::nullopt` for seq-correlated notifications).

## Current limits

Only the four types above have bodies. Types 20–37 (DescriptorUpdate, Submit, Assign, StateEvent,
reconcile) and 64–79 (log/service/artifact) are assigned IDs but **schema-pending** — they are
rejected with `unsupported_feature` until their schemas are defined. Role/direction enforcement
(which peer may send which type) lives in the session layer and is not yet implemented.

See [`proto/README.md`](proto/README.md) for regenerating the generated code after a schema edit.
