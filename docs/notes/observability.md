---
type: note
id: observability
status: active
maturity: growing
tags: [note, observability, logging]
aliases: ["Logging", "Structured logs", "Console"]
sources: ["../architecture/07-cross-cutting.md#73-observability"]
last_reviewed: 2026-11-25
---

# Observability

> Structured logs are the primary machine-readable record; a human-readable console is the
> interactive view. OS-specific logging is not the primary format.

## Structured log fields

Every component log should carry at least:

- timestamp
- component
- node ID (when applicable)
- execution ID (when applicable)
- severity
- event

## Metrics

Useful V1 metrics include:

- node CPU/RAM;
- queue length;
- scheduling latency;
- dispatch latency;
- service round-trip overhead.

Tracing is correlation by `ExecutionId`; no distributed tracing in V1.

## Log streams versus component logs

Execution stdout/stderr are ordinary timestamped byte streams with bounded history, replay then
live, and explicit gap reporting. They are not a required application logging API. Component logs
are the structured operational record.

## Related

- [Performance](performance.md)
- [Testing](testing.md)
- [Node](node.md)
- [Master](master.md)

## Canonical source

- [Cross-cutting: observability](../architecture/07-cross-cutting.md#73-observability)
- [Runtime: streams and logs](../spec/06-node-runtime-boundary.md#process-lifecycle-and-measurement)

## Open questions

- [OI-11 Log cursor and stream semantics](../questions/README.md#oi-11-log-cursor-and-stream-semantics)
