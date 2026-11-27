# Control codec benchmark

> Compares Protocol Buffers, FlatBuffers and Cap'n Proto on a representative `Submit` and
> `NodeDescriptor` control message. This is a decision input, not a stable wire schema.

## What it measures

- Encoded size.
- Encode latency (p50/p95, nanoseconds).
- Decode latency (p50/p95, nanoseconds).
- Typical and larger control-message shapes:
  - `Submit`: arguments, environment, labels, artifact inputs, outputs, services, capabilities,
    devices and strings.
  - `NodeDescriptor`: identity, resources, capabilities and devices.

The benchmark does not measure data-plane throughput, allocations, memory peaks or schema-evolution
behavior. Those remain follow-up checks before the wire format is frozen.

## Pinned dependencies

- FlatBuffers `v24.3.25`
- Protobuf `v3.21.12`
- Cap'n Proto `v1.0.2`

Dependencies are fetched with CMake `FetchContent` only when
`-DPLURIBUS_BUILD_CODEC_BENCH=ON` is set. The default build does not download them.

## Run

```sh
cmake --preset wsl-codec-bench
cmake --build --preset wsl-codec-bench --target pluribus_codec_bench
./build/wsl-codec-bench/benchmarks/codec/pluribus_codec_bench
```

From Windows, the parameterless wrapper is:

```bat
scripts\bench-codec.cmd
```

## Interpretation

- Control traffic is small and latency-tolerant relative to the 1 ms local-master target.
- Encoded size matters for many small messages; decode latency matters for high message rates.
- The benchmark uses generated code and library defaults; schema choices can change the results.
- A codec decision must also consider schema evolution, tooling, dependency maintenance and
  cross-platform support.
