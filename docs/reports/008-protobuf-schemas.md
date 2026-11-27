---
type: report
id: report-protobuf-schemas
status: review
task: Protobuf handshake schemas
branch: task/protobuf-schemas
base: task/dependency-enforcement
date: 2026-11-27
tags: [report, fabric, protobuf, wire, schema]
last_reviewed: 2026-11-27
---

# Task report: Protobuf handshake schemas

## Summary

Replaced the provisional binary bootstrap codec with versioned Protocol Buffers schemas for the
handshake. `pluribusfabric` now links `libprotobuf`; the generated `Hello`/`Welcome`/`Reject` code is
committed so the default build does not require `protoc`. The branch is stacked on the unmerged
dependency-enforcement/async/codec/session/transport branches.

## Scope

- **Branch:** `task/protobuf-schemas`
- **Base:** `task/dependency-enforcement` (stacked; merge order matters)
- **Commits:** see `git log --oneline task/dependency-enforcement..HEAD`
- **Date:** 2026-11-27
- **Status:** review

## What changed

- Added `fabric/proto/handshake.proto` with `Hello`, `Welcome`, `Reject`, `ProtocolVersion`,
  `SessionRole` and `RejectCode` messages.
- Committed generated `handshake.pb.h` / `handshake.pb.cc` (Protobuf `v3.21.12`).
- Rewrote `fabric/src/handshake_codec.cpp` to serialize/parse these messages; public API and
  validation bounds are unchanged.
- Added `protobuf` FetchContent to `pluribusfabric`:
  - `libprotobuf` only in default builds;
  - `protoc`/`libprotoc` enabled only when the codec benchmark is on (it reuses the same dependency).
- Updated the codec benchmark CMake to reuse the fabric-managed Protobuf dependency.
- Added `fabric/proto/README.md` with regeneration instructions.

## Verification

| Check | Command | Result |
|---|---|---|
| Windows debug | `cmake --build --preset windows-gcc-debug` + `ctest` | 4/4 passed |
| Windows release | `cmake --build --preset windows-gcc-release` + `ctest` | 4/4 passed |
| WSL debug | `scripts\test-wsl.cmd` | 3/3 passed |
| WSL release | `cmake --preset wsl-gcc-release` + build/test | 3/3 passed |
| Codec bench configure | `cmake --preset wsl-codec-bench` | passed (reuses protoc) |
| Documentation | `scripts\check-docs.cmd` | see below |
| Working tree | `git status --short` | clean after commit |

Documentation check result:

```text
Checked 83 Markdown files: 1215 relative links and 388 local anchors resolve.
```

## Measurements and benchmarks

**Not applicable as a new performance benchmark** — this is a schema/codec swap. The handshake
messages are small and the existing [transport benchmark](003-fabric-transport.md) covers frame
latency. One practical cost: the default build now fetches and builds `libprotobuf` on first
configure, which increases cold-build time; subsequent incremental builds are unaffected.

## Risks and limitations

- Generated Protobuf code is committed; any schema edit requires regeneration with `v3.21.12`.
- The default build now depends on `libprotobuf`; this is the accepted Q-47 direction but the vcpkg/
  dependency baseline (OI-27) is still open.
- Only the bootstrap handshake is schema-defined; the control-message envelope, heartbeat, service,
  error and artifact-location messages are still pending (OI-06).
- Protobuf headers trigger `-Wshadow` in our warning set, so `handshake_codec.cpp` disables that one
  warning locally rather than weakening the project-wide warning policy.
- The branch is stacked on unmerged branches; merge order matters.

## Follow-up work

1. Define the versioned control envelope and remaining control-message schemas.
2. Add tests for empty optional handshake fields (no token, no prior session).
3. Add a regeneration check or script so schema drift is caught.
4. Resolve the dependency/test baseline (OI-27) and pin Protobuf.
5. Add conformance fixtures before freezing the wire.

## Review checklist

- [ ] The `.proto` schema matches the handshake logical types.
- [ ] The public `handshake.h` API and validation bounds are unchanged.
- [ ] Committed generated code corresponds to `handshake.proto`.
- [ ] Default builds require `libprotobuf` but not `protoc`.
- [ ] The report matches the actual diff.
- [ ] Windows/WSL debug and release tests pass.
- [ ] No generated build output, secrets or host-specific paths are committed.
- [ ] `dev` and `master` were not modified directly.
- [ ] The user approves; only the user merges into `dev`.

## Links

- [Handshake schema](../../fabric/proto/handshake.proto)
- [Schema regeneration notes](../../fabric/proto/README.md)
- [Wire protocol](../spec/05-wire-protocol.md)
- [Q-47](../decisions/fabric-services.md#q-47)
- [OI-06 control schema](../questions/README.md#oi-06-control-message-schema-and-type-registry)
- [Implementation map](../maps/implementation.md)
- [Agent workflow](../../AGENTS.md)
