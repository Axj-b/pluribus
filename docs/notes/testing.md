---
type: note
id: testing
status: active
maturity: growing
tags: [note, testing, ci]
aliases: ["Test strategy", "Loopback tests"]
sources: ["../architecture/07-cross-cutting.md#75-testing-strategy", "../spec/09-repository-cmake-structure.md#tests-and-ci"]
last_reviewed: 2026-11-25
---

# Testing

> Test the boundaries that the architecture claims: pure model, fake transport/backend, real
> processes on loopback, then real multi-host nodes.

## Layers

| Layer | Scope |
|---|---|
| Unit | Core types, state machine, scheduler, framing, hashing, cache |
| Component | Fabric with loopback sockets; node with fake `HostBackend` |
| Integration | Real master + node process(es) on loopback |
| System | Real multi-host Windows/Linux/ARM64 with `plu run hello` |
| Fault injection | Kill node, drop connection, slow data plane, terminate not blocked |

## Determinism

- Use fake/in-memory transport so state-machine tests do not depend on sockets.
- Keep scheduler tests pure and independent of thread scheduling.
- Use exact integer/rational comparisons in scheduling tests; avoid epsilon assertions.
- Test malformed/oversized frames and path traversal explicitly.

## Multi-node on one host

Each node instance needs its own:

- persisted `NodeId` and node name (separate config/state directory);
- TOML config, cache root and sandbox root;
- control connection dialed to the master;
- distinct local service endpoint/port if it exposes one;
- fake capacity override and different configured capabilities for placement tests.

These tests exercise cross-node fabric paths, not the same-node shortcut. Real OS/architecture
heterogeneity still requires real machines.

## Current state

The repository uses a small dependency-free test harness while the vcpkg/GoogleTest baseline is
undecided. Unit tests cover core, scheduler, fabric framing and transport. Linux builds are exercised
locally through WSL with `scripts\test-wsl.cmd`. Integration/system harnesses do not exist yet.

## Related

- [Fabric](fabric.md)
- [Scheduler](scheduler.md)
- [Repository structure](repository-structure.md)
- [Implementation map](../maps/implementation.md)

## Canonical sources

- [Testing strategy](../architecture/07-cross-cutting.md#75-testing-strategy)
- [Tests and CI](../spec/09-repository-cmake-structure.md#tests-and-ci)
- [Multi-node on one host](../architecture/07-cross-cutting.md#751-multi-node-on-one-host-loopback-system-tests)

## Open questions

- [OI-27 Test and dependency baseline](../questions/README.md#oi-27-test-and-dependency-baseline)
- [OI-22 Dependency enforcement and toolchain pinning](../questions/README.md#oi-22-dependency-enforcement-and-toolchain-pinning)
