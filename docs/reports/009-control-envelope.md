---
type: report
id: report-control-envelope
status: review
task: Control envelope, type registry and initial message bodies
branch: task/control-envelope
base: dev
date: 2026-11-27
tags: [report, fabric, protobuf, wire, control]
last_reviewed: 2026-11-27
---

# Task report: Control envelope, type registry and initial message bodies

## Summary

Added the version-1 Protobuf control payload layer on top of the existing framing: a canonical
numeric `MessageType` registry, a portable `Error`, and an `Envelope` whose `oneof` carries the
first four implemented bodies (`RegisterNode`, `RegisterNodeAck`, `Heartbeat`, `HeartbeatAck`). The
public C++ API exposes plain core types and a bounded codec; generated Protobuf headers stay private
to `control_codec.cpp`. Bootstrap Hello/Welcome/Reject remain version 0 and are not wrapped. The
branch is ready for review; role/direction enforcement and session integration are deliberately out
of scope.

## Scope

- **Branch:** `task/control-envelope`
- **Base:** `dev`
- **Commits:** see `git log --oneline dev..HEAD`
- **Date:** 2026-11-27
- **Status:** review

## What changed

- Added `fabric/proto/control.proto` (`package pluribus.wire`):
  - the complete `MessageType` registry (bootstrap 1-3; control 16-37; services/artifacts 64-79;
    reserved gaps 4-15, 38-63, 80-255);
  - `ErrorCode` with stable explicit values starting at `UNSPECIFIED = 0`;
  - `CpuArchitecture`, `NodeHealth`, `NodeDescriptor`, `Error`;
  - the four implemented bodies and the `Envelope` (`request_id` plus a `oneof payload` with
    `error`, `register_node`, `register_node_ack`, `heartbeat`, `heartbeat_ack`).
- Committed generated `control.pb.h` / `control.pb.cc` (Protobuf `v3.21.12`, same as handshake).
- Added `fabric/include/pluribus/fabric/control.h`:
  - `enum class MessageType : std::uint8_t` projecting the registry;
  - the four plain body structs plus `ControlBody`/`ControlMessage`;
  - `encode_control`, `decode_control`, `response_type`, `is_known_message_type`,
    `is_implemented_message_type`.
- Added `fabric/src/control_codec.cpp`:
  - private `control.pb.h` include;
  - `static_assert`s tying every C++ `MessageType` value to `pluribus::wire::MESSAGE_TYPE_*`;
  - explicit `ErrorCode`, `CpuArchitecture` and `NodeHealth` mappings;
  - ordered decode validation (bootstrap/invalid type, known, implemented, size cap, parse,
    payload-case match, `request_id` presence, field bounds);
  - encode validation mirrored before serialization; serialization failure maps to `internal`.
- Added `tests/unit/fabric_control_tests.cpp` (13 tests) and wired it into `pluribus_fabric_tests`.
- Added `control.pb.cc` / `control_codec.cpp` to `pluribusfabric`; generated and codec sources use
  the same warning suppressions as handshake (`-w` / `/W0`, plus `-Wno-shadow` for the codec).
- Updated `fabric/proto/README.md`, the wire/node-master specs, OI-06, the fabric note,
  implementation/report maps, docs index, decision record Q-48 and this day's log.

### Deliberate deviations

- `RegisterNode`'s descriptor field is named `node_descriptor`, not `descriptor`. A Protobuf field
  named `descriptor` collides with the static `descriptor()` accessor that protoc 3.21 generates,
  so the schema cannot compile under that name.
- `decode_control` checks the invalid/bootstrap range (`type <= 3`) before the registry lookup. The
  brief listed "known type" first but also required bootstrap type 1 to return `invalid_argument`;
  these conflict, and this order satisfies the stated test outcomes. Unknown types such as 42 still
  return `unsupported_feature`.
- `response_type` covers request → response/ack pairs only. Notifications (`Heartbeat`,
  `WatchExecution`, `ServiceWatch`) return `nullopt`, as required by `response_type(18) == nullopt`.

## Verification

Environment: Windows MinGW GCC 13.2 (`windows-gcc-debug`, `windows-gcc-release`), WSL GCC 13.3
(`wsl-gcc-debug`).

| Check | Command | Result |
|---|---|---|
| Windows debug build | `cmake --build --preset windows-gcc-debug` | pass |
| Windows debug tests | `ctest --preset windows-gcc-debug --output-on-failure` | 4/4 targets pass |
| Windows release + Werror | `cmake --preset windows-gcc-release -DPLURIBUS_WARNINGS_AS_ERRORS=ON` + build | pass |
| WSL debug build/tests | `cmd /c scripts\test-wsl.cmd` | 3/3 targets pass |
| Unit counts | `pluribus_fabric_tests.exe` / core / scheduler | 33 / 22 / 16 passed, 0 failed |
| Docs | `cmd /c scripts\check-docs.cmd` | 84 files, 1235 links, 393 anchors resolve |
| Whitespace | `git diff --check` | no whitespace errors (one CRLF conversion warning only) |

## Measurements and benchmarks

**Not applicable** — this task adds a schema and codec; it does not change a benchmarked hot path.
No throughput, latency or memory measurement is claimed. Envelope size overhead (a tag/length plus
the 16-byte `request_id` where present) is inherent to the chosen Q-47 Protobuf direction.

## Risks and limitations

- Bodies for message types 20-79 (`DescriptorUpdate`, `Submit`, `Assign`, `StateEvent`, reconcile,
  log, service and artifact messages) are schema-pending and rejected with `unsupported_feature`;
  their OI dependencies are still open.
- Role/direction enforcement and session integration are not implemented. The codec validates
  structure only, not which peer may send a type.
- Unknown fields are tolerated on decode (proto3 default), but decoding into the core structs and
  re-encoding does not preserve them.
- `request_id` correlation is enforced only for the implemented request/response pair (16/17);
  notifications (18) and acks (19) must omit it.
- Generated code is committed; any schema edit requires regeneration with protoc `v3.21.12`.
- The registry is assigned but the wire contract is still not frozen (see the wire spec).

## Follow-up work

1. Define `DescriptorUpdate`/`Assign`/`StateEvent`/`Submit` and reconcile log/service/artifact bodies.
2. Add role/direction enforcement and session dispatch integration.
3. Add conformance fixtures and malformed-input corpus before freezing the wire.
4. Consider a regeneration check so `control.pb.*` cannot drift from `control.proto`.

## Review checklist

- [ ] The registry values and reserved ranges match the design review.
- [ ] The frame `type` byte is the only dispatch key; `Envelope` has no `type` or `version` field.
- [ ] Only the four agreed bodies are implemented.
- [ ] `control.h` does not expose generated Protobuf headers.
- [ ] Encode and decode validation bounds match the schema comments.
- [ ] Bootstrap messages are rejected inside a control envelope.
- [ ] The report matches the actual diff and the verification results were reproduced.
- [ ] No generated build output, secrets or host-specific paths are committed.
- [ ] `dev` and `master` were not modified; only the user merges.

## Links

- [Control schema](../../fabric/proto/control.proto)
- [Public control API](../../fabric/include/pluribus/fabric/control.h)
- [Codec](../../fabric/src/control_codec.cpp)
- [Tests](../../tests/unit/fabric_control_tests.cpp)
- [Wire protocol](../spec/05-wire-protocol.md)
- [Q-47 / Q-48](../decisions/fabric-services.md#q-48)
- [OI-06 control schema](../questions/README.md#oi-06-control-message-schema-and-type-registry)
- [Implementation map](../maps/implementation.md)
