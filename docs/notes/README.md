---
type: index
id: notes
status: active
tags: [note, index]
last_reviewed: 2026-11-25
---

# Notes

> Atomic orientation pages. Each note answers a narrow question and links to the canonical
> architecture/spec text. When a note and a spec disagree, the spec wins.

## Start with

- [Execution](execution.md) — the primitive everything else serves.
- [Execution lifecycle](execution-lifecycle.md) — states, `Lost`, reconciliation.
- [Scheduler](scheduler.md) — how placement works.
- [Fabric](fabric.md) — how components talk.
- [Artifact store](artifact-store.md) — how bytes move.

## Browse

- [Knowledge map](../maps/knowledge.md) — notes grouped by domain and tag.
- [Architecture map](../maps/architecture.md) — canonical architecture reading order.
- [Specification map](../maps/specifications.md) — exact contracts.

## All notes

| Note | Domain | Maturity |
|---|---|---|
| [Execution](execution.md) | execution | evergreen |
| [Execution lifecycle](execution-lifecycle.md) | execution | evergreen |
| [Scheduler](scheduler.md) | scheduling | evergreen |
| [Master](master.md) | control plane | growing |
| [Node](node.md) | node | growing |
| [Runtime](runtime.md) | runtime | growing |
| [Sandbox](sandbox.md) | runtime | growing |
| [Fabric](fabric.md) | fabric | growing |
| [Services](service.md) | services | growing |
| [Artifact store](artifact-store.md) | store | growing |
| [Packages](package.md) | store | growing |
| [Security](security.md) | cross-cutting | growing |
| [Observability](observability.md) | cross-cutting | growing |
| [Performance](performance.md) | cross-cutting | growing |
| [Testing](testing.md) | cross-cutting | growing |
| [Protocol versioning](protocol-versioning.md) | protocol | evergreen |
| [Repository structure](repository-structure.md) | engineering | growing |
| [CLI](cli.md) | product | growing |
| [Workspace cache](workspace-cache.md) | store | seed |

## How to add a note

Use [`templates/note.md`](../templates/note.md), then link it from at least one map and from
[the knowledge map](../maps/knowledge.md). See the [knowledge base guide](../knowledge-base.md).
