---
type: map
id: implementation-map
status: active
tags: [map, implementation, milestones]
aliases: ["Implementation MOC", "Current state"]
last_reviewed: 2026-11-25
---

# Implementation map

> What exists in code, what is next, and how to verify it. The canonical plan is the
> [roadmap](../architecture/08-roadmap.md); this map tracks execution against it.

## Current code map

| Target | Location | Status | Tests |
|---|---|---|---|
| `pluribuscore` | [`core/`](../../core) | Model, IDs, errors, paths, state machine | 22 unit tests |
| `pluribusfabric` | [`fabric/`](../../fabric) | Control framing, frame stream, in-memory loopback, blocking TCP transport | 15 unit tests |
| `pluribusscheduler` | [`master/scheduler/`](../../master/scheduler) | Pure V1 scheduler | 16 unit tests |
| `pluribusmaster` | — | Not started | — |
| `pluribusnode` | — | Not started | — |
| runtime/backends | — | Not started | — |
| `pluribusstore` | — | Not started | — |
| `pluribusclient` / `plu` | — | Not started | — |

## Milestone status

| Milestone | Scope | Status |
|---|---|---|
| A | Core model: IDs, spec, handle/state, node/resources, service, protocol contracts | Core model done; wire schema still open |
| B | Fabric: listener, connection, framing, serialization, heartbeat | Framing, frame stream, blocking TCP and loopback done; handshake, payload codec, heartbeat and async stack pending |
| C | Node registry: registration, CPU/RAM reporting, health, `plu nodes` | Not started |
| D | Remote execution: queue, trivial scheduler, assignment, launch, logs, exit, `plu run` | Scheduler done; master/node/runtime/CLI pending |
| E | Lifecycle: `plu ps`, `plu stop`, persistent executions, node loss | Not started |
| F | Services: register, resolve, proxy, lifetime binding | Not started |
| G | Artifacts: SHA-256 IDs, cache, transfer, materialization, outputs | Not started |

## Now / next / later

### Now

1. **Handshake and session scaffolding**: Hello/Welcome/Reject, version negotiation, directional
   sequence, replay/dedup boundary.
2. **Wire payload codec decision**: benchmark FlatBuffers, Cap'n Proto and Protobuf on representative
   control messages before freezing a schema. Until then, keep payloads opaque to framing.
3. **Dependency enforcement**: add a configure/CI check that compares the CMake target graph to the
   allowlist in [Spec 09](../spec/09-repository-cmake-structure.md#mechanical-enforcement).
4. **Dependency and test baseline**: decide vcpkg manifest/pinned baseline and replace or wrap the
   temporary dependency-free test harness.
5. **Async transport benchmark**: compare the current blocking transport and an async stack
   (for example Asio) against the documented loopback/LAN latency targets.

### Next

- `pluribusmaster`: node registry, heartbeat/health, FIFO queue, reservation ledger, execution
  manager, service registry stubs.
- `pluribusnode` with a fake `HostBackend`: register, assign, materialize, spawn, capture streams,
  report exit.
- Minimal executable/package resolution for `hello` plus a first `plu run hello` vertical slice.
- Structured logging with the required fields.

### Later

- Persistent executions, `plu ps`/`plu stop`, node-loss reconciliation.
- Services: provider registration, readiness, resolve, byte-stream connect.
- Artifact store: cache, transfer, materialization, output collection, workspace snapshots.
- Security hardening and real multi-host/ARM64 validation.

## Build and test

```sh
cmake --preset windows-gcc-debug
cmake --build --preset windows-gcc-debug
ctest --preset windows-gcc-debug
```

Linux/macOS presets are in [`CMakePresets.json`](../../CMakePresets.json). Warnings can be promoted
to errors with `-DPLURIBUS_WARNINGS_AS_ERRORS=ON`.

## Implementation constraints to remember

- Keep `core` free of platform headers and Pluribus dependencies.
- Do not infer execution state from connection state.
- Do not add application-domain types to core, master or scheduler.
- Do not choose a wire codec or package encoding by convenience without the benchmark/conformance
  decision.
- Treat the sandbox as operational isolation, not a security boundary.

## Open implementation questions

See the [open-question map](open-questions.md), especially:

- OI-06 control schema and numeric registry
- OI-08 authentication/identity
- OI-14 serializer selection
- OI-17 backend signatures and environment variables
- OI-21 license and vcpkg/DRIVE OS validation
- OI-22 dependency enforcement and pinned toolchain
- OI-28 minimal `hello` resolution

## Session history

- [2026-11-25](../log/2026-11-25.md) - core model, scheduler, fabric framing.
- [Fabric transport report](../reports/fabric-transport.md) - blocking TCP, loopback and frame stream.
- [Task workflow report](../reports/task-workflow.md) — branch/report process and review workflow.
