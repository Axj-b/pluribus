---
type: note
id: service
status: active
maturity: growing
tags: [note, services, routing]
aliases: ["Service model", "Provider", "Service resolution"]
sources: ["../spec/04-service-model.md", "../architecture/05-fabric-and-services.md"]
last_reviewed: 2026-11-25
---

# Services

> A service is a location-independent logical name such as `radarscape/sar/output`. A provider is a
> concrete instance exported by one execution. Applications resolve names, never physical topology.

## Model

- Names are hierarchical, slash-separated logical components with no host or port. The exact
  grammar is still open.
- A provider instance has a distinct ID/generation, logical name, owning execution, assignment, node,
  mode, readiness and internal routing metadata.
- Multiple providers may exist; V1 resolution returns at most one active **ready** provider.
- Declaring a service in an execution spec is intent, not readiness.
- The application marks a provider `READY` only after it can accept connections.
- A registration disappears automatically when its execution terminates.

## Resolution and transport

- `resolve(name, timeout)` waits for a matching resolvable provider or returns not-found/timeout;
  zero timeout is nonblocking.
- Resolution returns an opaque connection recipe and provider identity/generation, not a durable
  host address.
- V1 service transport is a reliable, ordered, bidirectional byte stream. Applications frame
  messages and implement request/reply themselves.
- A resolve can race with removal; re-resolve on failure or reconnect.
- Existing data streams may survive a master outage if their route is healthy; new global
  operations are not assumed to work.

## Failure behavior

- Not ready → wait/timeout; declaration alone is never returned.
- Provider exits/unregisters → remove it; existing streams close or fail.
- Node unreachable → unpublish, mark execution `Lost`, preserve evidence for reconciliation.
- Registry changes between resolve and connect → connect can fail; re-resolve.

## Related

- [Master](master.md)
- [Fabric](fabric.md)
- [Execution](execution.md)
- [Protocol versioning](protocol-versioning.md)

## Canonical sources

- [Service model](../spec/04-service-model.md)
- [Fabric and services](../architecture/05-fabric-and-services.md)
- [Node/master protocol: services](../spec/02-node-master-protocol.md#logs-services-and-artifacts)

## Open questions

- [OI-12 Service-name grammar](../questions/README.md#oi-12-service-name-grammar)
- [OI-13 Duplicate provider and readiness policy](../questions/README.md#oi-13-duplicate-provider-and-readiness-policy)
- [OI-07 Endpoint tickets and route authorization](../questions/README.md#oi-07-endpoint-tickets-and-route-authorization)
