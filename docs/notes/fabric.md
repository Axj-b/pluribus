---
type: note
id: fabric
status: active
maturity: growing
tags: [note, fabric, protocol, wire]
aliases: ["pluribusfabric", "Wire protocol", "Framing"]
sources: ["../architecture/05-fabric-and-services.md", "../spec/05-wire-protocol.md", "../spec/02-node-master-protocol.md"]
last_reviewed: 2026-11-25
---

# Fabric

> `pluribusfabric` is the communication substrate. The catalog separates small **control** messages
> from potentially large **data** streams so bulk traffic cannot block termination or heartbeats.

## Transport stance

- V1 uses TCP.
- One long-lived node-to-master control connection per node; separate client control connections.
- Separate on-demand data connections for artifacts, logs and service streams.
- Length-prefixed binary frames; language-neutral, never a dump of C++ layouts.
- Static master discovery in V1.
- Trusted development network; a join token is not encryption.

## Control framing

All integers are network byte order:

| Field | Bytes | Meaning |
|---|---:|---|
| `length` | 4 | Fixed 12-byte header plus payload |
| `version` | 1 | Wire major; `0` reserved for bootstrap |
| `type` | 1 | Message type within the negotiated major |
| `flags` | 2 | Known-flags validation |
| `sequence` | 8 | Directional session frame sequence; `0` before a session |
| `payload` | `length - 12` | Encoded message |

- The entire control frame is limited to 1 MiB, including the 4-byte length prefix.
- Reject `length < 12` or `length > 1,048,572` before allocation/decoding.
- No control fragmentation; large data belongs on the data plane.

## Handshake and session

- Bootstrap `Hello` / `Welcome` / `Reject` negotiate version, features, role, identity and optional
  resume cursors before application frames.
- Each direction has a frame sequence; acknowledgements mean "highest contiguous processed", not
  execution completion.
- Gaps require replay where retained, otherwise explicit resynchronization.
- Frame sequence, node `event_seq`, master execution revision, log cursor and service registry
  revision are independent domains.
- Mutating requests carry idempotency keys or naturally idempotent identities.

## Implementation status

`pluribusfabric` currently implements frame header encoding/decoding, bounded incremental decoding,
error latching, a blocking `FrameStream` over a narrow `Stream` interface, in-memory loopback
transport, blocking IPv4 TCP, a versioned Protobuf Hello/Welcome/Reject handshake codec, and a
`Session` that negotiates version/features/role/token and validates directional frame sequence.
Protocol Buffers is selected for V1 control payloads (Q-47). The numeric `MessageType` registry and
the control `Envelope` (request ID, portable `Error`, body `oneof`) are defined in
[`control.proto`](../../fabric/proto/control.proto), with `RegisterNode`, `RegisterNodeAck`,
`Heartbeat` and `HeartbeatAck` implemented; the frame `type` byte is the sole dispatch key and
bootstrap messages are not wrapped. A single-connection Asio benchmark was slower than the blocking
reference, so blocking remains the V1 reference until concurrency testing. Remaining bodies,
session replay/resume, heartbeat integration and the final async stack are next.

## Related

- [Using the control codec](../../fabric/README.md)
- [Protocol versioning](protocol-versioning.md)
- [Master](master.md)
- [Node](node.md)
- [Services](service.md)
- [Implementation map](../maps/implementation.md)

## Canonical sources

- [Fabric and services](../architecture/05-fabric-and-services.md)
- [Wire protocol](../spec/05-wire-protocol.md)
- [Node/master protocol](../spec/02-node-master-protocol.md)

## Open questions

- [OI-06 Control message schema and type registry](../questions/README.md#oi-06-control-message-schema-and-type-registry)
- [OI-14 Serializer selection](../questions/README.md#oi-14-serializer-selection)
- [OI-15 Data-plane framing and limits](../questions/README.md#oi-15-data-plane-framing-and-limits)
- [OI-25 Network stack selection](../questions/README.md#oi-25-network-stack-selection)
