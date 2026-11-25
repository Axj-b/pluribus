---
type: note
id: execution-lifecycle
status: active
maturity: evergreen
tags: [note, execution, lifecycle, state-machine]
aliases: ["Execution state machine", "Lost", "Lifecycle"]
sources: ["../spec/01-execution-model.md", "../spec/02-node-master-protocol.md"]
last_reviewed: 2026-11-25
---

# Execution lifecycle

> The master owns the authoritative state. Nodes report evidence. Connection state never proves
> process state.

## States

```text
Created -> Queued -> Assigned -> Materializing -> Starting -> Running
                                                              |-> Completed
                                                              |-> Failed
                                                              |-> Terminated
                                                              |-> Lost
```

Before `Running`, setup failures can move to `Failed` or `Terminated`. `Lost` means the outcome is
**unknown** after node/fabric failure and may be corrected by reconciliation.

## Rules

- `Completed`, `Failed` and `Terminated` are definitive terminal states.
- `Lost` is a provisional terminal observation, not proof of process death.
- A master **revision** is assigned to every published state change.
- A node emits a monotonic **event sequence per assignment**; the master deduplicates and detects gaps.
- Master revision, node event sequence and wire frame sequence are different domains.
- `wait` returns on a definitive terminal state **or** `Lost`; callers must inspect the outcome.
- A timeout is not an execution failure.
- Do not silently retry or migrate a `Lost` execution.

## Reconnect and reconciliation

- Nodes keep supervising processes through a master outage and buffer bounded events.
- On reconnect, a node reports active execution IDs, assignment identities, states and retained
  outcomes/services.
- A known matching execution may be adopted; a known `Lost` run with verifiable exit evidence may be
  corrected.
- Unknown/stale/conflicting reports are **orphans**. V1 defaults to keeping them locally supervised
  and non-resolvable until policy or reconciliation decides.
- An in-memory master restart can lose history; do not claim exactly-once submission across it.

## Logs

Node stdout/stderr are separate timestamped byte streams with bounded retained history (initially
about 10 MiB per execution). A subscriber gets retained history then live data; an evicted or
unprovable range must be reported as a gap, not silently skipped.

## Related

- [Execution](execution.md)
- [Node](node.md)
- [Master](master.md)
- [Fabric](fabric.md)
- [Services](service.md)

## Canonical sources

- [Execution model: state and events](../spec/01-execution-model.md#state-and-events)
- [Execution model: reconnect, orphans and master loss](../spec/01-execution-model.md#reconnect-orphans-and-master-loss)
- [Node/master protocol](../spec/02-node-master-protocol.md)
- [Wire sequencing](../spec/05-wire-protocol.md#sequencing-retry-and-resumption)

## Open questions

- [OI-05 Exit status and output collection](../questions/README.md#oi-05-exit-status-and-output-collection)
- [OI-03 Orphan policy and retention](../questions/README.md#oi-03-orphan-policy-and-retention)
- [OI-11 Log cursor and stream semantics](../questions/README.md#oi-11-log-cursor-and-stream-semantics)
