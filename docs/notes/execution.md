---
type: note
id: execution
status: active
maturity: evergreen
tags: [note, execution, core]
aliases: ["Execution", "ExecutionSpec"]
sources: ["../spec/01-execution-model.md", "../architecture/02-core-model.md"]
last_reviewed: 2026-11-25
---

# Execution

> An **execution** is one run of a native Pluribus-compatible executable. It is the only primitive
> the platform schedules; there are no `BuildJob`, `TestJob` or `RadarJob` types in core.

## Key points

- `ExecutionSpec` is immutable after acceptance. It describes executable source, arguments,
  environment, working directory, input/output artifacts, lifetime, services, resources,
  constraints and opaque labels.
- `request_id` is supplied by the client and reused unchanged on retries. It is not the execution
  identity.
- Executable source is explicit: package name, exact package version, immutable artifact, or a
  client-local file. There is no node `PATH` lookup or implicit source precedence.
- `lifetime` is `finite` or `persistent`; it describes intent, not restart or migration semantics.
- Resource requirements are optional. An absent value means **unknown**, not zero.
- Labels are opaque metadata for clients. The scheduler never interprets them.
- Submit returns a handle on acceptance, not on completion. A client disconnect does not terminate
  the execution.
- The scheduler sees only the spec plus node state and answer the question **where?** It never sees
  application-domain semantics.

## What an execution is not

- It is not a shell command line assembled by the node.
- It is not a container or a Linux process abstraction.
- It is not a build/test/radar-specific object.
- It is not owned by the TCP connection that submitted it.

## Related

- [Execution lifecycle](execution-lifecycle.md) — states, terminal outcomes, `Lost`.
- [Scheduler](scheduler.md) — how a compatible node is selected.
- [Node](node.md) and [Runtime](runtime.md) — how the host actually runs it.
- [Sandbox](sandbox.md) — the execution's logical workspace.
- [CLI](cli.md) — `plu run` and `--detach`.

## Canonical sources

- [Execution model](../spec/01-execution-model.md)
- [Core model: ExecutionSpec](../architecture/02-core-model.md#22-executionspec)
- [Execution state machine](../spec/01-execution-model.md#state-and-events)

## Open questions

- [OI-01 ExecutionId representation](../questions/README.md#oi-01-executionid-representation)
- [OI-02 Owner identity and lifetime](../questions/README.md#oi-02-owner-identity-and-lifetime)
- [OI-04 Portable native ABI](../questions/README.md#oi-04-portable-native-abi)
- [OI-05 Exit status and output collection](../questions/README.md#oi-05-exit-status-and-output-collection)
