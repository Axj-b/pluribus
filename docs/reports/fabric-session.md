---
type: report
id: report-fabric-session
status: review
task: Fabric bootstrap handshake and session sequencing
branch: task/fabric-session
base: task/fabric-transport
date: 2026-11-25
tags: [report, fabric, session, handshake]
last_reviewed: 2026-11-25
---

# Task report: Fabric bootstrap handshake and session sequencing

## Summary

Added the first session slice for `pluribusfabric`: bootstrap Hello/Welcome/Reject with a
provisional binary codec, version/feature/role/join-token negotiation, and a `Session` that assigns
and validates the directional post-handshake frame sequence. The transport report is still in
review, so this branch is stacked on `task/fabric-transport`.

## Scope

- **Branch:** `task/fabric-session`
- **Base:** `task/fabric-transport` (stacked; merge transport first)
- **Commits:** see `git log --oneline task/fabric-transport..HEAD`
- **Date:** 2026-11-25
- **Status:** review

## What changed

- Added `handshake.h`:
  - `SessionRole`, `ProtocolVersion`, `SessionLimits`, `SessionId`;
  - `Hello`, `Welcome`, `Reject` logical messages;
  - the server-side `HandshakePolicy`;
  - provisional binary encode/decode functions for bootstrap payloads.
- Added `session.h` / `session.cpp`:
  - `Session::accept` and `Session::connect`;
  - highest mutually supported version selection;
  - feature intersection and required-feature rejection;
  - optional role requirement and constant-time join-token comparison;
  - Welcome with generated session ID and negotiated limits;
  - post-handshake sequence assignment and validation;
  - explicit `SEQUENCE_GAP` / `CONFLICT` errors for ahead/duplicate sequences.
- Added 4 session tests:
  - handshake codec round-trip;
  - full handshake plus two sequenced request/response frames;
  - unsupported-version rejection;
  - invalid join-token rejection.
- Updated README, implementation/spec maps, fabric note and the implementation log.

## Verification

| Check | Command | Result |
|---|---|---|
| Debug build/tests | `cmake --build --preset windows-gcc-debug && ctest --preset windows-gcc-debug` | 3/3 targets passed |
| Release build/tests | `cmake --build --preset windows-gcc-release && ctest --preset windows-gcc-release` | 3/3 targets passed |
| WSL debug build/tests | `scripts\test-wsl.cmd` | 3/3 targets passed |
| WSL release build/tests | `cmake --preset wsl-gcc-release -DPLURIBUS_WARNINGS_AS_ERRORS=ON` + build/test | 3/3 targets passed |
| Unit test counts | core / scheduler / fabric executables | 22 / 16 / 19 passed |
| Documentation | `scripts\check-docs.cmd` | see below |
| Working tree | `git status --short` | clean after commit |

Documentation check result:

```text
Checked 77 Markdown files: 1149 relative links and 372 local anchors resolve.
```

## Measurements and benchmarks

**Not applicable as a new performance benchmark.** This slice is handshake and sequencing
correctness; the existing [fabric transport benchmark](fabric-transport.md) covers the underlying
loopback frame RTT and throughput. A future session benchmark should measure handshake latency and
sequenced request/response overhead after the payload codec is selected.

## Risks and limitations

- The bootstrap codec is **provisional and binary**, not the final serializer. It must be replaced
  after the OI-14 codec benchmark and OI-06 schema decision; do not treat its bytes as stable wire.
- Session replay/resume, acks, heartbeat and gap recovery are not implemented. A sequence gap or
  duplicate is an error; the connection must resync or close.
- The handshake identifies a peer with a string identity and optional shared token; this is not
  strong authentication. TLS and client authorization remain future hardening.
- The server handshake is blocking and has no explicit timeout; a silent peer can hold the
  connection thread.
- `Session` is single-reader/single-writer, consistent with `FrameStream`.
- The branch is stacked on the unmerged transport branch; merge order matters.

## Follow-up work

1. Run the serializer benchmark, select the codec and replace the provisional bootstrap codec.
2. Add session ack/replay/resume and explicit heartbeat policy.
3. Add handshake and session microbenchmarks after the codec choice.
4. Add request-correlation and idempotency scaffolding for the control messages.
5. Add connection/handshake timeouts and bounded outstanding-request enforcement.
6. Map handshake reject codes to the stable wire error registry.

## Review checklist

- [ ] Version/feature/role/token negotiation matches the protocol intent.
- [ ] Handshake frames use bootstrap version/sequence zero; post-handshake frames use the negotiated version and sequence starting at 1.
- [ ] Session ID and negotiated limits are propagated correctly.
- [ ] Sequence errors are explicit and do not apply the frame.
- [ ] The report matches the actual diff.
- [ ] No generated build output, secrets or host-specific paths are committed.
- [ ] `dev` and `master` were not modified directly.
- [ ] The user approves; only the user merges into `dev`.

## Links

- [Fabric note](../notes/fabric.md)
- [Transport report](fabric-transport.md)
- [Implementation map](../maps/implementation.md)
- [OI-06 control schema](../questions/README.md#oi-06-control-message-schema-and-type-registry)
- [OI-14 serializer selection](../questions/README.md#oi-14-serializer-selection)
- [Agent workflow](../../AGENTS.md)
