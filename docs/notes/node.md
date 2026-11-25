---
type: note
id: node
status: active
maturity: growing
tags: [note, node, agent]
aliases: ["pluribusnode", "Node agent"]
sources: ["../architecture/04-node-and-runtime.md", "../spec/06-node-runtime-boundary.md"]
last_reviewed: 2026-11-25
---

# Node

> `pluribusnode` is the per-host agent. It is deliberately simple: register, report, materialize,
> launch, supervise, stream, stop and collect. It has no understanding of why an execution exists.

## Responsibilities

- Connect/register with the master and heartbeat.
- Report static descriptor and dynamic resource/health state.
- Receive assignments and validate assignment identity.
- Materialize inputs and create the sandbox.
- Ask the runtime to spawn the process with explicit arguments, workdir and clean environment.
- Capture stdout/stderr separately with timestamps and bounded history.
- Monitor the process tree; map graceful terminate/kill to host mechanisms.
- Collect declared outputs and report the outcome.
- Expose service endpoints and register/readiness/unregister with the master.
- Maintain the local artifact cache and announce/evict locations.

## Failure behavior

- On master loss, keep supervising existing processes, buffer bounded events, accept no new global
  work and reconnect.
- Never equate control-connection loss with process termination.
- On reconnect, report active IDs, assignment identities, state and retained outcomes so the master
  can reconcile; unknown/stale reports become orphans under configured policy.
- Repeated stop/terminate commands are safe; report incomplete tree handling rather than pretending
  success.

## Configuration

TOML in V1, foreground operation. Persist a generated UUID as `NodeId`; the human-readable name is
separate and changeable. Relative config paths resolve against the config file's directory.
Capabilities are auto-detected where reliable and supplemented by configuration; host OS is not a
normal scheduling capability.

## Related

- [Runtime](runtime.md)
- [Sandbox](sandbox.md)
- [Master](master.md)
- [Execution lifecycle](execution-lifecycle.md)
- [Artifact store](artifact-store.md)

## Canonical sources

- [Node and runtime](../architecture/04-node-and-runtime.md)
- [Node/runtime boundary](../spec/06-node-runtime-boundary.md)
- [Node/master protocol](../spec/02-node-master-protocol.md)

## Open questions

- [OI-17 Host backend signatures and environment](../questions/README.md#oi-17-host-backend-signatures-and-environment)
- [OI-03 Orphan policy and retention](../questions/README.md#oi-03-orphan-policy-and-retention)
- [OI-11 Log cursor and stream semantics](../questions/README.md#oi-11-log-cursor-and-stream-semantics)
