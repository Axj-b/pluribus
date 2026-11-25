---
title: "07. Scheduler V1"
type: spec
id: spec-07
status: draft
tags: [spec, scheduler]
last_reviewed: 2026-11-25
---

# 07. Scheduler V1

This document defines a deterministic V1 scheduling policy. The scheduler is a pure function;
queueing, atomic reservation and dispatch are master responsibilities. See
[execution model](01-execution-model.md), [node/master protocol](02-node-master-protocol.md),
[runtime](06-node-runtime-boundary.md), and [store](08-artifact-store-protocol.md).

## Inputs and result

Conceptual immutable types, not public C++ signatures:

```text
Request = { compatibleKeys: set<(abiVersion, cpuArch)>, requiredCapabilities: set<string>,
            requiredDevices: set<string>, explicitNode: optional<NodeId>,
            cpu: optional<millicores>, memory: optional<bytes>, inputArtifacts: set<ArtifactId> }
Node = { id: NodeId, abiVersion, cpuArch, capabilities, devices,
         totalCpu: millicores, totalMemory: bytes, online, healthy, draining,
         measuredCpuLoad: optional<basisPoints>, cpuObservedAt: optional<time>,
         availableMemory: optional<bytes>, memoryObservedAt: optional<time>,
         reservedCpu: millicores, reservedMemory: bytes, cachedArtifacts: set<ArtifactId> }
Policy = { defaultCpu: millicores, defaultMemory: bytes, localNode: optional<NodeId>,
           snapshotTime: time, maxMeasurementAge: duration }
Result = Selected{nodeId, reserveCpu, reserveMemory} |
         TemporarilyUnschedulable{reason} | FundamentallyIncompatible{reason}
schedule(Request, list<Node>, Policy, optional<HistoricalProfile>) -> Result
```

The snapshot contains all registered nodes, including offline/draining nodes, and active
reservations. `compatibleKeys` comes from the resolved executable/package, intersected with any
explicit architecture/ABI constraints. Capabilities and devices are exact opaque names, with no
application-specific interpretation. Execution labels are **never read or interpreted**; they
are only metadata. Host OS is not an eligibility capability. Resource declarations are optional:
an absent value is unknown, not zero and not an implicit one CPU. Substitute the corresponding
positive master-configured default reservation independently for each missing value; explicit
values must be nonnegative and policy/defaults valid. Invalid requests/configuration are rejected
before scheduling. The default amounts are configuration, not protocol constants.
Measurements and observation times must be from the same master clock domain; a sample is fresh
only when its timestamp is not in the future and its age is at most `maxMeasurementAge` at the
supplied `snapshotTime`. CPU load is quantized to integer basis points (10,000 is 100%); CPU
capacity/reservations use integer millicores and memory uses integer bytes. Arithmetic and
comparisons retain exact rational values with sufficiently wide checked intermediates.

`HistoricalProfile` is an explicit future hook, ignored in V1. Measurements may be recorded for
future policy, but V1 neither predicts requirements nor changes reservations using history.

## Filter, admission and score

1. **Static compatibility:** retain registered nodes matching at least one ABI/architecture key,
   explicit node if present, all mandatory capabilities/devices, and whose total CPU and memory
   can hold the requested/default reservation. No translation or emulation is implied.
2. If none remain, return `FundamentallyIncompatible` relative to the **current registered
   inventory**, not a claim that no future node can ever match. The master policy chooses to
   leave it queued for a node/configuration change or fail it with an incompatibility reason.
3. Retain online, healthy, non-draining nodes. Require fresh CPU and available-memory measurements;
   absent/stale measurements do not mean zero load or unlimited capacity. If none remain, return
   `TemporarilyUnschedulable`.
4. For each node compute `usedCpu = measuredCpuLoad / 10000 * totalCpu + reservedCpu` and
   `usedMemory = totalMemory - availableMemory + reservedMemory`. Reservations may overlap
   observed usage; adding both deliberately double-counts overlap conservatively because V1
   cannot reliably identify which measurements already include which assignments. In particular,
   several pending assignments cannot all consume the same measured headroom. Clamp valid
   measurements to their physical range before calculating; reject malformed or zero totals.
   Admit only if `usedCpu + reserveCpu <= totalCpu` and `usedMemory + reserveMemory <=
   totalMemory`. If none fit, return `TemporarilyUnschedulable`. This conservative estimate can
   delay work unnecessarily and cannot guarantee hard limits against unexpected actual usage.
5. Score admitted nodes by projected normalized load:
   `score = ((usedCpu + reserveCpu) / totalCpu + (usedMemory + reserveMemory) / totalMemory) / 2`.
   Lowest wins. On exact equality prefer `localNode` if one is among the tied nodes; otherwise
   choose the lexicographically smallest canonical UUID. Use deterministic rational/integer
   comparison, not epsilon-dependent floating point ties.

The `cachedArtifacts` field is a locality hook: V1 does not alter eligibility, score, or tie-breaks
based on it. A future policy may use cache information without changing execution semantics.
Optional preferred capabilities likewise have no effect in V1; mandatory requirements are filters.

## Pseudocode and queue ownership

```text
function schedule(request, nodes, policy, history):
    reservation = each requested quantity or its configured default
    static = nodes matching keys, explicit node, requirements and total capacity
    if static.empty(): return FundamentallyIncompatible("no registered static match")
    live = static where online and healthy and not draining and measurements fresh
    candidates = []
    for node in live:
        used = measured_usage(node) + reservations(node)  // conservative overlap
        if used + reservation <= total(node):
            candidates.append((projected_normalized_score(node, used, reservation), node))
    if candidates.empty(): return TemporarilyUnschedulable("unavailable, unknown or busy")
    winner = minimum by (score, local_tie_key, canonical_node_uuid)
    return Selected(winner.id, reservation.cpu, reservation.memory)
```

The second sort key is `0` for the configured local node and `1` for every other node. The pure
function does not reserve anything. Master processes a strict FIFO queue: examine the head; on
selection atomically revalidate the snapshot and commit its reservation with the assignment before
dispatch; on stale state retry against a fresh snapshot. Only then consider the next head. On
temporary failure leave the head queued until a relevant change; later entries do not bypass it.
For fundamental incompatibility, configured policy either fails/removes the head or leaves it
queued, also blocking later entries. This intentionally permits head-of-line blocking, no priority
or preemption. Re-evaluate on resource, health, registration, cancellation or policy changes.

Each reservation belongs to one assignment and is committed once, including while materializing
or starting; duplicate dispatch/ack must not double count. Release once on confirmed terminal
outcome, cancellation before launch, or definitive dispatch failure. On a partition, do not treat
an uncertain process as confirmed stopped: keep accounting associated with its assignment until
reconciliation/orphan resolution, and do not schedule onto the unavailable node. On reconnect
reconstruct/reconcile reservations without counting the same execution twice. Do not retry the
execution elsewhere automatically merely because it was marked `Lost`.

## Worked examples

For illustration only, defaults are 1 core and 1 GiB. Both nodes are healthy, same ABI/arch,
8 cores and 16 GiB; no existing reservations. A is local, with CPU load 25% and 12 GiB free;
B has CPU load 12.5% and 14 GiB free. For an unspecified request, A's projected score is
`((2+1)/8 + (4+1)/16)/2 = 0.34375`; B's is `((1+1)/8 + (2+1)/16)/2 = 0.21875`.
B wins despite A being local. If B has A's exact measurements, A wins the tie. If both are
busy and cannot admit the reservation, the head remains queued. If neither has a requested ABI,
the result is fundamental incompatibility and the master applies its queue/fail policy.

## Unit-test cases

- ABI/architecture mismatches, missing mandatory capability/device and explicit node mismatch;
  host OS and arbitrary label changes do not alter results.
- Empty inventory, offline-only static match, draining node and stale/missing metrics distinguish
  fundamental from temporary outcomes; later compatible registration makes a queued item eligible.
- Unknown CPU and memory independently use configured defaults; explicit override, zero, invalid
  negative input and total-capacity-too-small cases are checked.
- Boundaries at exactly full capacity admit; over capacity rejects. Overlapping measurements and
  reservations are conservatively summed; several pending assignments cannot reuse headroom.
  Projected scores use both resources.
- Lowest score beats locality; equal score prefers local, then UUID; input list order does not
  affect selection. Cache entries, historical profiles and labels do not change V1 results.
- FIFO head blocking, policy-controlled incompatible failure, snapshot race/retry, duplicate
  assignment, release-once and partition/reconciliation accounting are master integration tests.

## Open issues

- Select deployment defaults for reservations and measurement freshness; validate this deliberately
  conservative accounting against real workloads before introducing resource enforcement.
- Define any future locality or historical-profile policy without silently changing V1 ordering.

## Knowledge base links

- Map: [Specification map](../maps/specifications.md)
- Notes: [Scheduler](../notes/scheduler.md)
- Decisions: [Decision records](../decisions/README.md), [Decision map](../maps/decisions.md)
- Open questions: [Open questions register](../questions/README.md)
