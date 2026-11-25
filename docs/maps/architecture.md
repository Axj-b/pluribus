---
type: map
id: architecture-map
status: active
tags: [map, architecture]
aliases: ["Architecture MOC"]
last_reviewed: 2026-11-25
---

# Architecture map

> Start here to understand the shape of Pluribus. The canonical detail is in
> [`architecture/`](../architecture/01-overview.md); the notes below are entry points.

## Reading order

1. [Overview](../architecture/01-overview.md) — vision, invariants, layering.
2. [Core model](../architecture/02-core-model.md) — identities, execution, state, node/service descriptors.
3. [Control plane](../architecture/03-control-plane.md) — master, scheduler, health, failure.
4. [Node and runtime](../architecture/04-node-and-runtime.md) — agent, `pluribussim`, backend, sandbox.
5. [Fabric and services](../architecture/05-fabric-and-services.md) — transport, protocol, service routing.
6. [Store and packaging](../architecture/06-store-and-packaging.md) — artifacts, cache, packages.
7. [Cross-cutting](../architecture/07-cross-cutting.md) — security, observability, performance, tests, config.
8. [Roadmap](../architecture/08-roadmap.md) — layout, milestones, prototypes, non-goals.

## Layers

```text
Application
  -> Pluribus public API / protocol
     -> pluribusmaster (control plane)
        <-> pluribusfabric
           <-> pluribusnode
              -> pluribussim
                 -> host backend
                    -> host OS + hardware
```

The application decides **what**; the scheduler decides **where**; the runtime decides **how on this
host**; the fabric decides **how endpoints communicate**. See the
[overview invariant](../architecture/01-overview.md#18-roles-in-the-invariant).

## Invariants

- Execution is the only primitive; no domain-specific job types in core.
- The scheduler has no application-domain knowledge.
- Host OS stays below `pluribussim`; public values do not leak host handles or paths.
- Architecture (x86_64/arm64) is explicit; no emulation in V1.
- Distribution happens at execution boundaries, not shared memory.
- Dependency direction is strict: core depends on nothing Pluribus-specific.
- Failure is explicit: `Lost` is not proof of process death, and connection state is not execution state.
- Compatibility is ABI + CPU architecture, not host OS.

## Component notes

| Component | Note | Canonical |
|---|---|---|
| Execution | [Execution](../notes/execution.md) | [Spec 01](../spec/01-execution-model.md) |
| Lifecycle | [Execution lifecycle](../notes/execution-lifecycle.md) | [Spec 01](../spec/01-execution-model.md) |
| Master | [Master](../notes/master.md) | [Architecture 03](../architecture/03-control-plane.md) |
| Scheduler | [Scheduler](../notes/scheduler.md) | [Spec 07](../spec/07-scheduler-v1.md) |
| Node | [Node](../notes/node.md) | [Architecture 04](../architecture/04-node-and-runtime.md) |
| Runtime | [Runtime](../notes/runtime.md) | [Spec 06](../spec/06-node-runtime-boundary.md) |
| Sandbox | [Sandbox](../notes/sandbox.md) | [Architecture 04](../architecture/04-node-and-runtime.md#44-execution-sandbox) |
| Fabric | [Fabric](../notes/fabric.md) | [Spec 05](../spec/05-wire-protocol.md) |
| Services | [Services](../notes/service.md) | [Spec 04](../spec/04-service-model.md) |
| Artifacts | [Artifact store](../notes/artifact-store.md) | [Spec 08](../spec/08-artifact-store-protocol.md) |
| Packages | [Packages](../notes/package.md) | [Spec 08](../spec/08-artifact-store-protocol.md#package-model) |
| CLI | [CLI](../notes/cli.md) | [Architecture 08](../architecture/08-roadmap.md#cli-plu) |
| Security | [Security](../notes/security.md) | [Spec 10](../spec/10-security-model.md) |
| Observability | [Observability](../notes/observability.md) | [Architecture 07](../architecture/07-cross-cutting.md#73-observability) |
| Performance | [Performance](../notes/performance.md) | [Architecture 07](../architecture/07-cross-cutting.md#74-performance-targets) |
| Testing | [Testing](../notes/testing.md) | [Architecture 07](../architecture/07-cross-cutting.md#75-testing-strategy) |
| Versioning | [Protocol versioning](../notes/protocol-versioning.md) | [Spec 05](../spec/05-wire-protocol.md) |
| Repository | [Repository structure](../notes/repository-structure.md) | [Spec 09](../spec/09-repository-cmake-structure.md) |

## Dependencies

```text
core <- fabric
core <- store
core <- runtime <- platform backends
core <- scheduler
core + fabric + store + scheduler <- master
core + fabric + store + runtime <- node
core + fabric <- client <- cli
```

Forbidden: `core -> platform`, `master -> RadarScape`, `scheduler -> application domain`,
`fabric -> RadarScape`. See [Spec 09](../spec/09-repository-cmake-structure.md).

## Decisions and questions

- [Decision map](decisions.md)
- [Open questions](open-questions.md)
- [Implementation map](implementation.md)
