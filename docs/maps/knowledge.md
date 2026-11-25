---
type: map
id: knowledge-map
status: active
tags: [map, note, index]
aliases: ["Note index", "Knowledge MOC"]
last_reviewed: 2026-11-25
---

# Knowledge map

> All atomic notes, grouped by domain. Each note is a short orientation page that links to the
> canonical architecture/spec text.

## Execution and lifecycle

| Note | Tags | One-line summary |
|---|---|---|
| [Execution](../notes/execution.md) | `execution`, `core` | The only scheduling primitive: one run of a compatible native executable. |
| [Execution lifecycle](../notes/execution-lifecycle.md) | `execution`, `lifecycle`, `state-machine` | Master-owned states, terminal outcomes, `Lost` reconciliation. |

## Control plane

| Note | Tags | One-line summary |
|---|---|---|
| [Master](../notes/master.md) | `master`, `control-plane` | Registry, queue, scheduling, ownership, service and artifact indexes. |
| [Scheduler](../notes/scheduler.md) | `scheduler` | Pure compatibility filter, admission, exact score and deterministic tie-break. |

## Node and runtime

| Note | Tags | One-line summary |
|---|---|---|
| [Node](../notes/node.md) | `node`, `agent` | Registers, supervises local executions, streams logs, reconciles after outages. |
| [Runtime](../notes/runtime.md) | `runtime`, `abi`, `backend` | `pluribussim`, clean environment, host-backend boundary. |
| [Sandbox](../notes/sandbox.md) | `sandbox`, `paths` | Per-execution logical workspace; operational, not security-grade isolation. |

## Fabric and services

| Note | Tags | One-line summary |
|---|---|---|
| [Fabric](../notes/fabric.md) | `fabric`, `protocol`, `wire` | Control/data planes, framing, handshake, sequencing, retry. |
| [Services](../notes/service.md) | `services`, `routing` | Logical names, providers, readiness, byte-stream connections. |
| [Protocol versioning](../notes/protocol-versioning.md) | `protocol`, `versioning` | Wire, ABI and package versions are separate contracts. |

## Store and packaging

| Note | Tags | One-line summary |
|---|---|---|
| [Artifact store](../notes/artifact-store.md) | `store`, `artifacts`, `cache` | Content-addressed blobs, manifests, pins, transfer, outputs. |
| [Packages](../notes/package.md) | `packages`, `abi` | `.plp` manifests and per-ABI/architecture native entries. |
| [Workspace cache](../notes/workspace-cache.md) | `store`, `workspace`, `proposal` | Proposed missing-blob negotiation and workspace snapshots. |

## Cross-cutting

| Note | Tags | One-line summary |
|---|---|---|
| [Security](../notes/security.md) | `security`, `trust` | Trusted-network V1, admissions, validation, residual risk. |
| [Observability](../notes/observability.md) | `observability`, `logging` | Structured logs, metrics, tracing, CLI output. |
| [Performance](../notes/performance.md) | `performance`, `targets` | Explicit latency/throughput targets and measurement discipline. |
| [Testing](../notes/testing.md) | `testing`, `ci` | Unit/component/integration/system layers and fault injection. |

## Engineering and product

| Note | Tags | One-line summary |
|---|---|---|
| [Repository structure](../notes/repository-structure.md) | `repository`, `cmake`, `dependencies` | Targets, allowed edges, enforcement and presets. |
| [CLI](../notes/cli.md) | `cli`, `api`, `ux` | `plu` commands, public-API dogfooding, config precedence. |

## By tag

- `abi` — [Packages](../notes/package.md), [Protocol versioning](../notes/protocol-versioning.md), [Runtime](../notes/runtime.md)
- `artifacts` — [Artifact store](../notes/artifact-store.md), [Workspace cache](../notes/workspace-cache.md)
- `cache` — [Artifact store](../notes/artifact-store.md), [Workspace cache](../notes/workspace-cache.md)
- `cli` — [CLI](../notes/cli.md)
- `cmake` — [Repository structure](../notes/repository-structure.md)
- `execution` — [Execution](../notes/execution.md), [Execution lifecycle](../notes/execution-lifecycle.md)
- `fabric` — [Fabric](../notes/fabric.md)
- `master` — [Master](../notes/master.md)
- `node` — [Node](../notes/node.md)
- `protocol` — [Fabric](../notes/fabric.md), [Protocol versioning](../notes/protocol-versioning.md)
- `scheduler` — [Scheduler](../notes/scheduler.md)
- `security` — [Security](../notes/security.md)
- `services` — [Services](../notes/service.md)
- `testing` — [Testing](../notes/testing.md)

## See also

- [Maps of content](README.md)
- [Architecture map](architecture.md)
- [Specification map](specifications.md)
