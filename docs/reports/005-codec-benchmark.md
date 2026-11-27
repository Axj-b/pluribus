---
type: report
id: report-codec-benchmark
status: merged
task: Control codec benchmark and selection
branch: task/codec-benchmark
base: task/fabric-session
merged_into: dev
merged_at: 2026-11-27
date: 2026-11-26
tags: [report, codec, benchmark, protobuf, flatbuffers, capnproto]
last_reviewed: 2026-11-26
---

# Task report: Control codec benchmark and selection

## Summary

Benchmarked Protocol Buffers, FlatBuffers and Cap'n Proto on representative `Submit` and
`NodeDescriptor` control messages and selected **Protocol Buffers for V1 control payloads** (Q-47).
Protobuf is materially smaller on typical messages and its latency remains far below the control
targets at V1 scale. The branch is stacked on the unmerged fabric session/transport branches.

## Scope

- **Branch:** `task/codec-benchmark`
- **Base:** `task/fabric-session` (stacked; merge transport and session first)
- **Commits:** see `git log --oneline task/fabric-session..HEAD`
- **Date:** 2026-11-26
- **Status:** merged

## What changed

- Added `benchmarks/codec/` with:
  - equivalent `Submit` and `NodeDescriptor` schemas for all three codecs;
  - pinned FetchContent dependencies;
  - encode/decode harness with typical and large payload shapes;
  - methodology README.
- Added the `wsl-codec-bench` CMake preset and parameterless
  `scripts/bench-codec.ps1` / `scripts/bench-codec.cmd`.
- Added decision Q-47: Protocol Buffers for V1 control payloads.
- Resolved OI-14 and updated the wire/cross-cutting notes, decision map, implementation map and log.
- Added this report to the reports index and map.

## Verification

| Check | Command | Result |
|---|---|---|
| Benchmark build/run | `scripts\bench-codec.cmd` | completed; three runs collected |
| Windows build/tests | `cmake --build --preset windows-gcc-debug` + `ctest` | 3/3 targets passed |
| Windows release | `cmake --build --preset windows-gcc-release` + `ctest` | 3/3 targets passed |
| WSL build/tests | `scripts\test-wsl.cmd` | 3/3 targets passed |
| Documentation | `scripts\check-docs.cmd` | see below |
| Working tree | `git status --short` | clean after commit |

Documentation check result:

```text
Checked 79 Markdown files: 1176 relative links and 384 local anchors resolve.
```

## Measurements and benchmarks

Environment:

- WSL2 Ubuntu 24.04, GCC 13.3.0, CMake Release `-O3 -DNDEBUG`.
- FlatBuffers `v24.3.25`, Protobuf `v3.21.12`, Cap'n Proto `v1.0.2`.
- Three benchmark runs; table values are medians of the per-run p50/p95 figures.
- `Submit` typical: 8 args, 8 env vars, 4 labels, 4 inputs, 2 outputs; large: 64/64/16/64/16 with
  longer strings.
- `NodeDescriptor` typical: 8 capabilities, 2 devices; large: 64 capabilities, 16 devices.
- Iterations: 3000 (typical), 1000 (large) per encode/decode.

| Message | Profile | Codec | Bytes | Encode p50/p95 (µs) | Decode p50/p95 (µs) |
|---|---|---|---:|---:|---:|
| Submit | typical | Protobuf | 1051 | 2.27 / 3.93 | 1.91 / 2.10 |
| Submit | typical | FlatBuffers | 1728 | 0.88 / 0.90 | 0.24 / 0.24 |
| Submit | typical | Cap'n Proto | 1632 | 0.80 / 0.93 | 0.11 / 0.12 |
| Submit | large | Protobuf | 27865 | 30.29 / 49.10 | 20.73 / 33.17 |
| Submit | large | FlatBuffers | 32504 | 6.85 / 10.37 | 1.67 / 2.11 |
| Submit | large | Cap'n Proto | 33048 | 4.27 / 5.68 | 0.25 / 0.26 |
| NodeDescriptor | typical | Protobuf | 268 | 0.52 / 0.55 | 0.49 / 0.81 |
| NodeDescriptor | typical | FlatBuffers | 464 | 0.20 / 0.21 | 0.06 / 0.06 |
| NodeDescriptor | typical | Cap'n Proto | 480 | 0.21 / 0.30 | 0.07 / 0.07 |
| NodeDescriptor | large | Protobuf | 5414 | 3.05 / 4.64 | 2.81 / 4.90 |
| NodeDescriptor | large | FlatBuffers | 6232 | 1.07 / 1.70 | 0.16 / 0.22 |
| NodeDescriptor | large | Cap'n Proto | 6496 | 0.65 / 0.67 | 0.07 / 0.07 |

Interpretation:

- Protobuf is **~1.55× smaller** than Cap'n Proto/FlatBuffers on typical `Submit` and **~1.17×
  smaller** on large `Submit`; NodeDescriptor shows the same direction.
- Cap'n Proto is the latency leader, especially on decode; FlatBuffers is close on encode but not
  better enough to offset its larger encoding.
- Protobuf's worst measured case is ~30 µs encode / ~21 µs decode for a large submit. That is still far
  below the 1 ms local-master target and the expected V1 control rate.
- At V1 scale (2–10 nodes, well below 1000 concurrent executions), schema evolution, tooling,
  cross-language support and encoded size matter more than the last microseconds of decode.
- Protobuf also has the strongest established story for unknown-field preservation and generated-code
  tooling, which the wire-evolution contract needs.

**Recommendation:** Protocol Buffers for V1 control. Revisit Cap'n Proto if control message rates or
zero-copy decode become a measured bottleneck; revisit the full schema after conformance and
allocation benchmarks.

## Risks and limitations

- The benchmark covers only two message shapes; heartbeat, service resolve/watch, errors, artifact
  location and assignment are not measured yet.
- Allocations, peak memory, compression and schema-evolution behavior are not measured.
- Only WSL2/Linux x86_64 was measured; native Windows, ARM64 and DRIVE OS results are unavailable.
- FetchContent builds are slow and not part of the default build; the benchmark is a separate,
  dependency-heavy target.
- Protobuf generated code and runtime add a dependency; the vcpkg/dependency baseline is still open.
- The branch is stacked on unmerged fabric branches; merge order matters.
- The selected Protobuf schemas are not defined yet; Q-47 selects the codec, not the schema.

## Follow-up work

1. Define versioned Protobuf control schemas and replace the provisional bootstrap codec.
2. Add heartbeat, service, error and artifact-location cases to the benchmark.
3. Measure allocations, peak memory and schema evolution/unknown fields.
4. Add native Windows and ARM64 runs when toolchains are available.
5. Decide the dependency baseline (OI-27) and pin Protobuf in CMake/vcpkg.
6. Add conformance fixtures before declaring the wire stable.

## Review checklist

- [ ] The benchmark schemas are equivalent across codecs.
- [ ] The measurement method and environment are clear.
- [ ] The recommendation follows from the data rather than preference.
- [ ] Limitations and missing cases are explicit.
- [ ] The report matches the actual diff.
- [ ] `scripts\bench-codec.cmd`, Windows tests and WSL tests pass.
- [ ] No generated build output, secrets or host-specific paths are committed.
- [ ] `dev` and `master` were not modified directly.
- [ ] The user approves; only the user merges into `dev`.

## Links

- [Codec benchmark harness](../../benchmarks/codec/README.md)
- [Q-47](../decisions/fabric-services.md#q-47)
- [OI-14 serializer selection](../questions/README.md#oi-14-serializer-selection)
- [Wire protocol](../spec/05-wire-protocol.md#serialization-benchmark-and-selection)
- [Implementation map](../maps/implementation.md)
- [Agent workflow](../../AGENTS.md)
