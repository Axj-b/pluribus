---
type: note
id: scheduler
status: active
maturity: evergreen
tags: [note, scheduler, control-plane]
aliases: ["Scheduler V1", "Placement"]
sources: ["../spec/07-scheduler-v1.md"]
last_reviewed: 2026-11-25
---

# Scheduler

> The V1 scheduler is a **pure function**: given a request, node snapshots and policy, it returns a
> `NodeId` (plus the reservation to commit), or a temporary/fundamental failure. It reserves
> nothing and dispatches nothing.

## Inputs

- **Request:** compatible ABI/architecture keys, required capabilities/devices, explicit node,
  optional CPU/memory, input artifacts.
- **Node snapshot:** identity, ABI/arch, capabilities/devices, totals, online/healthy/draining,
  measured CPU load and available memory with timestamps, existing reservations, cached artifacts.
- **Policy:** default CPU/memory reservations, local node, snapshot time, maximum measurement age.

## Filter, admission, score

1. **Static compatibility** — at least one ABI/arch key, explicit node if set, all mandatory
   capabilities/devices, and total capacity can hold the reservation.
2. If no static match → `FundamentallyIncompatible` relative to the current inventory.
3. Keep only online, healthy, non-draining nodes with fresh measurements.
4. Compute conservative used CPU/memory (measured usage **plus** reservations, deliberately
   double-counting overlap). Admit only if the reservation fits.
5. Score projected normalized load; lowest wins. Exact tie → local node, then lexicographically
   smallest canonical UUID.

## Deliberate V1 choices

- Strict FIFO in the master; a blocked head is not bypassed.
- No priority, preemption, predictive model, historical profile, cache-based placement, or label
  interpretation.
- Resource declarations are optional; master defaults apply per missing value.
- Host OS is not a scheduling capability.
- Measurements that are absent, stale or in the future do not mean zero load or unlimited capacity.
- Arithmetic is exact integer/rational, not epsilon-dependent floating point.

## Implementation

`pluribusscheduler` implements the pure policy with a dependency-free 128/256-bit arithmetic helper
and unit tests for the documented cases. Queueing, reservation commit and dispatch remain master
work.

## Related

- [Master](master.md)
- [Execution](execution.md)
- [Node](node.md)
- [Implementation map](../maps/implementation.md)

## Canonical source

- [Scheduler V1](../spec/07-scheduler-v1.md)

## Open questions

- [OI-18 Scheduler defaults and measurement freshness](../questions/README.md#oi-18-scheduler-defaults-and-measurement-freshness)
