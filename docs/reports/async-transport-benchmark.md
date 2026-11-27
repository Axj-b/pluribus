---
type: report
id: report-async-transport-benchmark
status: review
task: Async transport benchmark
branch: task/async-transport-bench
base: task/codec-benchmark
date: 2026-11-27
tags: [report, fabric, transport, asio, benchmark]
last_reviewed: 2026-11-27
---

# Task report: Async transport benchmark

## Summary

Benchmarked a standalone Asio async TCP echo path against the existing blocking `FrameStream`
reference on WSL2 loopback. On a single connection, Asio was slower for both small-frame RTT and
large-frame throughput in this environment. The blocking reference remains the V1 control-path
baseline for now; a concurrent-connection benchmark is still needed before selecting an async stack.

## Scope

- **Branch:** `task/async-transport-bench`
- **Base:** `task/codec-benchmark` (stacked; merge order matters)
- **Commits:** see `git log --oneline task/codec-benchmark..HEAD`
- **Date:** 2026-11-27
- **Status:** review

## What changed

- Added `benchmarks/transport/asio_bench.cpp`:
  - standalone Asio `io_context` echo server/client;
  - C++20 coroutines and `asio::use_awaitable`;
  - same control-frame encoding and payload sizes as the blocking benchmark;
  - small-frame RTT and large-frame throughput modes.
- Added `benchmarks/transport/CMakeLists.txt` with pinned Asio `asio-1-30-2`.
- Added the `wsl-transport-bench` preset and parameterless
  `scripts/bench-transport.ps1` / `scripts/bench-transport.cmd`.
- Updated the scripts README and implementation map; added this report and the log entry.

## Verification

| Check | Command | Result |
|---|---|---|
| Asio benchmark build/run | `scripts\bench-transport.cmd` | completed; four runs collected |
| Windows build/tests | `cmake --build --preset windows-gcc-debug` + `ctest` | 3/3 targets passed |
| Windows release | `cmake --build --preset windows-gcc-release` + `ctest` | 3/3 targets passed |
| WSL build/tests | `scripts\test-wsl.cmd` | 3/3 targets passed |
| Documentation | `scripts\check-docs.cmd` | see below |
| Working tree | `git status --short` | clean after commit |

Documentation check result:

```text
Checked 81 Markdown files: 1193 relative links and 386 local anchors resolve.
```

## Measurements and benchmarks

Environment:

- WSL2 Ubuntu 24.04, GCC 13.3.0, CMake Release (`-O3 -DNDEBUG`).
- Loopback IPv4 (`127.0.0.1`), one connection, sequential round trips.
- Same 256-byte small frame and 1,048,560-byte large payload as the blocking benchmark.
- Asio: four runs; blocking reference: three runs from the [fabric transport report](fabric-transport.md).

| Metric | Blocking reference | Asio | Difference |
|---|---:|---:|---:|
| Small-frame RTT p50 | 63.6 µs | 81.0 µs | Asio ~1.27× slower |
| Small-frame RTT p95 | 87.9 µs | 129.0 µs | Asio ~1.47× slower |
| Stream throughput | 5147 MiB/s | 828 MiB/s | Asio ~6.2× lower |

Asio raw samples:

```text
asio_tcp_small_frame_rtt_us p50=68.669 p95=118.34 n=1000 payload=256
asio_tcp_stream_throughput_mib_s=925.275 payload=1048560 frames=64

asio_tcp_small_frame_rtt_us p50=76.76 p95=141.46 n=1000 payload=256
asio_tcp_stream_throughput_mib_s=852.896 payload=1048560 frames=64

asio_tcp_small_frame_rtt_us p50=85.19 p95=118.56 n=1000 payload=256
asio_tcp_stream_throughput_mib_s=802.913 payload=1048560 frames=64

asio_tcp_small_frame_rtt_us p50=91.25 p95=139.474 n=1000 payload=256
asio_tcp_stream_throughput_mib_s=720.241 payload=1048560 frames=64
```

Interpretation:

- For a single in-flight connection, the Asio event loop adds measurable overhead compared with
  blocking sockets.
- This does **not** disprove the value of an async stack: the test does not exercise many concurrent
  connections, partial writes, cancellation or backpressure. Those are the cases where an async
  reactor is expected to matter.
- The result argues against replacing the blocking reference just for single-connection latency or
  loopback throughput.

**Recommendation:** keep the blocking `Stream`/`FrameStream` reference for the V1 control path and
the narrow transport interface. Do not adopt Asio into `pluribusfabric` yet. Add a concurrent-client
benchmark (for example 32–128 simultaneous connections) and a LAN test before making the final async
stack decision.

## Risks and limitations

- Single connection only; no concurrency, cancellation, partial-write or backpressure coverage.
- WSL2 only; native Windows, Linux and ARM64 results are unavailable.
- No allocation, peak-memory or CPU-time measurements.
- Asio benchmark uses the same frame codec but its server/client structure differs from `FrameStream`.
- The branch is stacked on the unmerged codec/session/transport branches; merge order matters.
- The async transport decision remains open (OI-25).

## Follow-up work

1. Add a concurrent Asio benchmark with 32–128 clients and mixed payload sizes.
2. Add a comparable blocking multi-connection reference (thread pool or one thread per connection).
3. Measure allocations, CPU time and peak memory.
4. Add LAN and native Windows/ARM64 runs.
5. Decide whether Asio (or another stack) replaces the blocking transport after the concurrency data.
6. Keep the `Stream` boundary unchanged until that decision.

## Review checklist

- [ ] The Asio and blocking measurements use equivalent frame sizes and round-trip behavior.
- [ ] Single-connection limitations are explicit.
- [ ] The recommendation does not overclaim from one workload.
- [ ] The report matches the actual diff.
- [ ] `scripts\bench-transport.cmd`, Windows tests and WSL tests pass.
- [ ] No generated build output, secrets or host-specific paths are committed.
- [ ] `dev` and `master` were not modified directly.
- [ ] The user approves; only the user merges into `dev`.

## Links

- [Asio benchmark source](../../benchmarks/transport/asio_bench.cpp)
- [Blocking transport report](fabric-transport.md)
- [OI-25 network stack selection](../questions/README.md#oi-25-network-stack-selection)
- [Fabric note](../notes/fabric.md)
- [Implementation map](../maps/implementation.md)
- [Agent workflow](../../AGENTS.md)
