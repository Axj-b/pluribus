---
type: note
id: performance
status: active
maturity: growing
tags: [note, performance, targets]
aliases: ["Performance targets", "Latency targets"]
sources: ["../architecture/07-cross-cutting.md#74-performance-targets"]
last_reviewed: 2026-11-25
---

# Performance

> Use explicit targets so architectural regressions are visible. Do not optimize artifact throughput
> before real build/RadarScape workloads are measured.

## V1 targets

| Metric | Target |
|---|---|
| Local master request overhead | < 1 ms |
| LAN scheduling + control dispatch on 1 GbE | < 10 ms |
| Small persistent-service round trip, extra Pluribus overhead | < 2 ms |
| Node heartbeat | 1 s default, configurable |
| Node considered unavailable | 5 missed heartbeats by default, configurable |

## Measurement discipline

- Automate regression benchmarks for these targets.
- Separate control-plane latency from bulk data throughput.
- Record hardware, compiler, build type, workload, warmup and distributions.
- Do not let serializer benchmarks be dominated by data-plane throughput.
- Prefer measuring real workloads over speculative optimization.

## Related

- [Fabric](fabric.md)
- [Scheduler](scheduler.md)
- [Observability](observability.md)
- [Testing](testing.md)

## Canonical source

- [Cross-cutting: performance targets](../architecture/07-cross-cutting.md#74-performance-targets)
- [Wire benchmark methodology](../spec/05-wire-protocol.md#serialization-benchmark-and-selection)
