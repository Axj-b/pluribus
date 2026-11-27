---
type: report
id: report-fabric-transport
status: review
task: Fabric transport boundary
branch: task/fabric-transport
base: dev
date: 2026-11-25
tags: [report, fabric, transport, benchmark]
last_reviewed: 2026-11-25
---

# Task report: Fabric transport boundary

## Summary

Implemented the first transport slice for `pluribusfabric`: a narrow blocking `Stream` interface,
an in-memory loopback transport for deterministic tests, blocking IPv4 TCP listener/connect
transports for Windows and POSIX, and a `FrameStream` that reads/writes control frames over a stream.
The slice follows the accepted OI-25 suggestion to prove the transport with a blocking reference
implementation before choosing an async stack.

## Scope

- **Branch:** `task/fabric-transport`
- **Base:** `dev`
- **Commits:** see `git log --oneline dev..HEAD`
- **Date:** 2026-11-25
- **Status:** review

## What changed

- Added `pluribus::fabric::Stream` as the blocking byte-stream boundary.
- Added `make_loopback_streams()` for deterministic component tests.
- Added `TcpListener::bind`, `TcpListener::accept` and `connect_tcp`:
  - IPv4 only for this slice;
  - explicit bind address, ephemeral port support;
  - Winsock implementation on Windows, POSIX sockets elsewhere;
  - `TCP_NODELAY` enabled;
  - portable `Error` mapping instead of leaking socket errors.
- Added `FrameStream`:
  - encodes and writes one control frame;
  - incrementally reads and decodes frames;
  - reports a truncated frame as `INVALID_ARGUMENT`;
  - returns an empty optional on clean EOF.
- Added 5 transport tests:
  - loopback byte round-trip and EOF;
  - frame round-trip over loopback;
  - truncated frame detection;
  - real TCP loopback frame exchange;
  - invalid bind address rejection.
- Added `tests/bench/fabric_transport_bench.cpp`:
  - small-frame TCP round-trip latency;
  - TCP stream throughput;
  - in-memory frame throughput.
- Added WSL/Linux presets (`wsl-gcc-debug`, `wsl-gcc-release`) and parameterless
  `scripts/test-wsl.ps1` / `scripts/test-wsl.cmd` helpers.
- Updated README, implementation/spec maps, fabric/testing notes and the implementation log.
- Documented the MinGW/GCC 13 `std::make_shared`/static-libstdc++ workaround in `loopback.cpp`.

## Verification

| Check | Command | Result |
|---|---|---|
| Debug build/tests | `cmake --build --preset windows-gcc-debug && ctest --preset windows-gcc-debug` | 3/3 targets passed |
| Release build/tests | `cmake --build --preset windows-gcc-release && ctest --preset windows-gcc-release` | 3/3 targets passed |
| WSL debug build/tests | `scripts\test-wsl.cmd` | 3/3 targets passed |
| WSL release build/tests | `cmake --preset wsl-gcc-release -DPLURIBUS_WARNINGS_AS_ERRORS=ON` + build/test | 3/3 targets passed |
| Unit test counts | core / scheduler / fabric executables | 22 / 16 / 15 passed |
| Benchmarks | `build\windows-gcc-release\tests\pluribus_fabric_bench.exe` and WSL release binary | see below |
| Documentation | `scripts\check-docs.cmd` | see below |
| Working tree | `git status --short` | clean after commit |

Documentation check result:

```text
Checked 76 Markdown files: 1138 relative links and 370 local anchors resolve.
```

## Measurements and benchmarks

Environment:

- Windows development host, loopback IPv4 (`127.0.0.1`).
- MinGW GCC 13.2.0, CMake Release (`-O3 -DNDEBUG`).
- WSL2 Ubuntu 24.04, GCC 13.3.0, CMake Release (`-O3 -DNDEBUG`), Unix Makefiles.
- Three benchmark runs per platform; values below are median with range.
- Small-frame RTT: 100 warmup + 1000 round trips, 256-byte payload.
- Throughput: 64 frames of 1,048,560 bytes with a final acknowledgement.

### Windows

| Metric | Median | Range |
|---|---:|---:|
| TCP small-frame RTT p50 | 18 µs | 18–21 µs |
| TCP small-frame RTT p95 | 35 µs | 19–37 µs |
| TCP stream throughput | 1381 MiB/s | 1032–1471 MiB/s |
| In-memory frame throughput | 620 MiB/s | 608–621 MiB/s |

### Linux / WSL2

| Metric | Median | Range |
|---|---:|---:|
| TCP small-frame RTT p50 | 63.6 µs | 62.5–64.0 µs |
| TCP small-frame RTT p95 | 87.9 µs | 87.7–97.6 µs |
| TCP stream throughput | 5147 MiB/s | 5055–5671 MiB/s |
| In-memory frame throughput | 1000 MiB/s | 978–1012 MiB/s |

Raw samples:

```text
tcp_small_frame_rtt_us p50=18 p95=19 n=1000 payload=256
tcp_stream_throughput_mib_s=1032.36 payload=1048560 frames=64
memory_frame_throughput_mib_s=620.717 payload=1048560 frames=64

tcp_small_frame_rtt_us p50=18 p95=35 n=1000 payload=256
tcp_stream_throughput_mib_s=1471.38 payload=1048560 frames=64
memory_frame_throughput_mib_s=620.182 payload=1048560 frames=64

tcp_small_frame_rtt_us p50=21 p95=37 n=1000 payload=256
tcp_stream_throughput_mib_s=1380.96 payload=1048560 frames=64
memory_frame_throughput_mib_s=607.865 payload=1048560 frames=64
```

Linux / WSL2 raw samples:

```text
tcp_small_frame_rtt_us p50=63.969 p95=97.628 n=1000 payload=256
tcp_stream_throughput_mib_s=5055.42 payload=1048560 frames=64
memory_frame_throughput_mib_s=999.642 payload=1048560 frames=64

tcp_small_frame_rtt_us p50=62.489 p95=87.853 n=1000 payload=256
tcp_stream_throughput_mib_s=5146.8 payload=1048560 frames=64
memory_frame_throughput_mib_s=977.88 payload=1048560 frames=64

tcp_small_frame_rtt_us p50=63.629 p95=87.667 n=1000 payload=256
tcp_stream_throughput_mib_s=5670.55 payload=1048560 frames=64
memory_frame_throughput_mib_s=1011.89 payload=1048560 frames=64
```

Interpretation:

- Small control frames are comfortably below the 1 ms local-master target on both hosts.
- The Linux RTT is higher on this WSL2 host, likely due to virtualization/scheduling overhead; it is
  still well below the target and is not a native-Linux or LAN result.
- The TCP throughput number is a loopback ceiling, not a LAN or disk-bound artifact result.
- The in-memory result is dominated by framing/copy and loopback synchronization; it is a regression
  baseline, not a promise about network throughput.
- Variance is high enough that the range should be used for comparisons, not a single run.

## Risks and limitations

- The transport is a **blocking reference spike**, not the final async design. The event-driven
  requirement and backpressure policy are still open (OI-25).
- No handshake, session sequencing, replay, heartbeat or payload schema exists yet.
- IPv4 only; IPv6 and DNS resolution are not implemented.
- No TLS, authentication, connection limits, read/write timeouts or health checks.
- `FrameStream` is single-reader/single-writer; concurrent use needs external synchronization.
- TCP listener uses `SO_REUSEADDR`; this is a development convenience, not a security policy.
- Benchmarks are loopback-only and cover one Windows host and one WSL2 host; they do not cover native
  Linux, LAN, ARM64 or DRIVE OS.
- WSL2 adds virtualization/scheduling overhead; Linux numbers from WSL are useful for correctness and
  relative regressions, not absolute native-Linux latency.
- The MinGW/GCC 13 workaround avoids `std::make_shared`; revisit when the toolchain is pinned.

## Follow-up work

1. Add Hello/Welcome/Reject and session sequence/replay scaffolding.
2. Choose and benchmark the async transport stack against this blocking reference.
3. Add read/write timeouts and bounded per-connection queues/backpressure.
4. Add connection limits and explicit shutdown tests.
5. Decide whether IPv6 belongs in the first control transport.
6. Add a LAN benchmark when a second host is available.

## Review checklist

- [ ] The `Stream` boundary is narrow enough to swap the transport.
- [ ] TCP bind/connect behavior is explicit and portable.
- [ ] Frame read/write behavior and EOF/truncation errors are correct.
- [ ] The benchmark methodology is clear and the numbers are plausible.
- [ ] The report matches the actual diff.
- [ ] No generated build output, secrets or host-specific paths are committed.
- [ ] `dev` and `master` were not modified directly.
- [ ] The user approves; only the user merges into `dev`.

## Links

- [Fabric note](../notes/fabric.md)
- [Testing note](../notes/testing.md)
- [Implementation map](../maps/implementation.md)
- [Task reports index](../reports/README.md)
- [OI-25 network stack selection](../questions/README.md#oi-25-network-stack-selection)
- [Agent workflow](../../AGENTS.md)
