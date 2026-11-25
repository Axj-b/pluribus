---
type: note
id: master
status: active
maturity: growing
tags: [note, master, control-plane]
aliases: ["pluribusmaster", "Control plane"]
sources: ["../architecture/03-control-plane.md", "../spec/02-node-master-protocol.md"]
last_reviewed: 2026-11-25
---

# Master

> `pluribusmaster` is the authoritative control plane for one Pluribus machine. It decides where
> work runs; it does not run application work itself unless its host also runs a node.

## Responsibilities

- Node registry and health.
- Execution queue and placement.
- Execution ownership and authoritative state/revisions.
- Service registry.
- Artifact location index.
- Client sessions and request/deduplication state.
- Global logical namespace.

## V1 stance

- Single authoritative master; no clustering or consensus.
- In-memory state initially, with a storage abstraction for later durability.
- Strict FIFO queue; a blocked head is not bypassed.
- Reservations are committed once per assignment and released once on a confirmed terminal outcome,
  cancellation before launch, or definitive dispatch failure.
- On partition, uncertain executions keep their accounting until reconciliation; do not schedule
  onto an unavailable node or automatically retry elsewhere.
- Static master configuration; discovery is optional later.
- If the master dies, existing processes can continue locally, but no new scheduling occurs. State
  may be lost/reconstructible in V1.

## What the master must not know

- RadarScape, CMake, SAR, AI, tests, NVIDIA, or any application domain.
- How a host executes a process; that belongs to node/runtime.
- A physical node's OS as a scheduling capability.

## Related

- [Scheduler](scheduler.md)
- [Execution lifecycle](execution-lifecycle.md)
- [Node](node.md)
- [Fabric](fabric.md)
- [Services](service.md)
- [Artifact store](artifact-store.md)

## Canonical sources

- [Control plane](../architecture/03-control-plane.md)
- [Node/master protocol](../spec/02-node-master-protocol.md)
- [Wire protocol](../spec/05-wire-protocol.md)

## Open questions

- [OI-09 Request deduplication and durable master state](../questions/README.md#oi-09-request-deduplication-and-durable-master-state)
- [OI-08 Authentication and client identity](../questions/README.md#oi-08-authentication-and-client-identity)
- [OI-01 ExecutionId representation](../questions/README.md#oi-01-executionid-representation)
