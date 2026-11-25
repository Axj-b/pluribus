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

## 7.6 Configuration

Master and node use TOML configuration (Q-22). Contents: bind interface, master address, node
capabilities, cache size/path, sandbox root, log level and master policies. Decision (Q-44):
precedence is CLI option > environment > user config > system/default config.

## 7.7 Versioning & compatibility

Separate numbers: protocol version (wire), ABI version (runtime contract for apps), package format
version. Handshake rejects incompatible protocol; scheduler filters by ABI.
