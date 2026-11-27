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
| `pluribusfabric` | [`fabric/`](../../fabric) | Framing, frame stream, loopback, blocking TCP, bootstrap handshake and session sequencing | 19 unit tests |
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
| B | Fabric: listener, connection, framing, serialization, heartbeat | Framing, transport, bootstrap handshake and session sequencing done; payload codec, replay/resume, heartbeat and async stack pending |
| C | Node registry: registration, CPU/RAM reporting, health, `plu nodes` | Not started |
| D | Remote execution: queue, trivial scheduler, assignment, launch, logs, exit, `plu run` | Scheduler done; master/node/runtime/CLI pending |
| E | Lifecycle: `plu ps`, `plu stop`, persistent executions, node loss | Not started |
| F | Services: register, resolve, proxy, lifetime binding | Not started |
| G | Artifacts: SHA-256 IDs, cache, transfer, materialization, outputs | Not started |

## Now / next / later

### Now

1. **Define the versioned Protobuf control schemas** and replace the provisional bootstrap codec.
2. **Session replay/resume and heartbeat**: acks, replay buffers, gap recovery, heartbeat cadence.
3. **Dependency enforcement**: the configure-time graph check and include scan are done; add
   negative fixtures and pin the toolchain/dependency baseline.
4. **Dependency and test baseline**: decide vcpkg manifest/pinned baseline and replace or wrap the
   temporary dependency-free test harness.
5. **Concurrent transport benchmark and stack decision**: the single-connection Asio benchmark is
   done and favors the blocking reference; add 32–128 concurrent clients before selecting the async
   stack.

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
- [Fabric transport report](../reports/003-fabric-transport.md) - blocking TCP, loopback and frame stream.
- [Fabric session report](../reports/004-fabric-session.md) - bootstrap handshake and sequenced sessions.
- [Codec benchmark report](../reports/005-codec-benchmark.md) - Protocol Buffers selected for V1 control.
- [Async transport benchmark report](../reports/006-async-transport-benchmark.md) - blocking reference vs Asio on single-connection loopback.
- [Dependency enforcement report](../reports/007-dependency-enforcement.md) - CMake graph check and platform-include scan.
- [Task workflow report](../reports/001-task-workflow.md) — branch/report process and review workflow.
