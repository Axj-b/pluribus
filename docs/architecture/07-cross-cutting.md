---
title: "7. Cross-cutting concerns"
type: architecture
id: arch-07
status: draft
tags: [architecture, security, observability, performance, testing, configuration]
last_reviewed: 2026-11-25
---

# 7. Cross-cutting concerns

## 7.1 Technology choices (V1)

| Area | Choice | Notes |
|------|--------|-------|
| Language | C++23 | MSVC/Clang on Windows, Clang/GCC on Linux, Apple Clang/Clang on macOS, NVIDIA-supported DRIVE OS toolchain on Thor; pin versions in CI (Q-38) |
| Build | CMake + Ninja | Per-platform backend selection at configure time |
| Serialization | Benchmark before selection | FlatBuffers, Cap'n Proto and Protocol Buffers, using representative control messages (Q-15) |
| Hashing | SHA-256 | Use a maintained library, not custom primitives (Q-39) |
| Transport | TCP | Benchmark-driven async/network stack (Q-39, Q-40) |
| Concurrency | Async event-driven I/O | Coroutines acceptable; small pool for blocking/CPU work; no thread-per-connection or massive custom actor framework; hide behind boundaries (Q-40) |
| Tests | Unit + process-level integration + two-node loopback system tests | Multi-host CI later |

Decision (Q-39): Prefer small maintained dependencies, including established crypto, TLS, hashing and
serialization libraries. Choose a reproducible CMake package strategy before the repository grows.

## 7.2 Security (V1)

Assumption: trusted development machines on a trusted network. Not safe for untrusted code.

Decision (Q-41): Implement node join token, explicit bind interface, protocol version validation,
message-size limits, path-traversal protection,
execution-directory confinement, clean environment construction, explicit artifact hashes, node
identity.

Later: TLS, node certificates, signed packages, Windows Job Objects, Linux namespaces/cgroups/seccomp,
macOS sandbox, privilege separation. Broader networks, including vehicle Wi-Fi, should eventually be
treated as untrusted.

Threat notes: a node executes arbitrary binaries by design; the master therefore is effectively
remote code execution as a service — bind to LAN interface only and document it.

## 7.3 Observability

Decision (Q-42): Structured logs with timestamp, component, node ID, execution ID, severity and
event; human-readable console output for interactive use. OS-specific logging is not the primary
format. Metrics: node CPU/RAM, queue length, scheduling latency, dispatch
latency, service RTT overhead. Tracing: correlation by ExecutionId; no distributed tracing in V1.

## 7.4 Performance targets

| Metric | Target |
|--------|--------|
| Local master request overhead | < 1 ms |
| LAN scheduling + control dispatch (1 GbE) | < 10 ms |
| Small persistent-service round trip, extra overhead | < 2 ms |
| Heartbeat | 1 s default, master-configurable (Q-18) |
| Node unavailable | 5 missed heartbeats by default, master-configurable (Q-18) |

Artifact throughput is not optimized until real workloads are measured. Benchmarks for these targets
should be automated as regression tests.

## 7.5 Testing strategy

- Unit: core types, state machine, scheduler (pure), framing, hashing, cache.
- Component: fabric with loopback sockets, node with fake `HostBackend`.
- Integration: master + node(s) as real processes on loopback ("two-node loopback").
- System: real multi-host (Windows/Linux/ARM64) with `plu run hello`.
- Fault injection: kill node, drop connection, slow data plane (verify terminate not blocked).
- Fake/in-memory transport to keep the state machine tests deterministic.

### 7.5.1 Multi-node on one host (loopback system tests)

Several `pluribusnode` processes plus one `pluribusmaster` run as separate processes on one machine.
Each node instance needs its own:

| Aspect | Requirement |
|--------|-------------|
| Identity | Separate persisted `NodeId` (UUID) and node name; therefore a separate config/state directory per instance |
| Config | Separate TOML file with its own cache location and sandbox root, so nodes never share artifacts or execution directories |
| Connection | Own control connection dialed to the master; no extra ports needed unless the node exposes a service endpoint, in which case each needs a distinct port or local endpoint name |

Caveats:
- **Resource reporting:** all nodes see the same host CPU/RAM, so reports overlap. Scheduler tests
  use a config override that declares a smaller fake capacity per node.
- **Capabilities:** give each test node different config-defined capabilities (e.g. `target.thor`)
  to exercise placement.
- **Routing:** nodes on one host still communicate through the fabric, not the same-node shortcut,
  so these tests exercise the cross-node paths. The same-node shortcut needs its own test.
- **Not covered:** real heterogeneity (different OSes, CPU architectures, networks) still requires
  real machines.

The test harness should create a temporary directory per node (config, state, cache, sandbox),
allocate free ports, start and stop processes, and clean up on failure.

## 7.6 Configuration

Master and node use TOML configuration (Q-22). Contents: bind interface, master address, node
capabilities, cache size/path, sandbox root, log level and master policies. Decision (Q-44):
precedence is CLI option > environment > user config > system/default config.

## 7.7 Versioning & compatibility

Separate numbers: protocol version (wire), ABI version (runtime contract for apps), package format
version. Handshake rejects incompatible protocol; scheduler filters by ABI.

## Knowledge base links

- Map: [Architecture map](../maps/architecture.md)
- Notes: [Security](../notes/security.md), [Observability](../notes/observability.md), [Performance](../notes/performance.md), [Testing](../notes/testing.md)
- Decisions: [Decision records](../decisions/README.md), [Decision map](../maps/decisions.md)
- Open questions: [Open questions register](../questions/README.md)
