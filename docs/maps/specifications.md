---
type: map
id: specifications-map
status: active
tags: [map, spec]
aliases: ["Specification MOC"]
last_reviewed: 2026-11-25
---

# Specification map

> The specs are the precise contracts. Read them in priority order when building; use the notes
> for orientation and the decision records for why.

## Priority order

| # | Specification | Status | Related notes | Implemented |
|---:|---|---|---|---|
| 1 | [Execution model](../spec/01-execution-model.md) | Draft | [Execution](../notes/execution.md), [Lifecycle](../notes/execution-lifecycle.md) | Model and state machine in `pluribuscore` |
| 2 | [Node/master protocol](../spec/02-node-master-protocol.md) | Draft | [Master](../notes/master.md), [Node](../notes/node.md), [Fabric](../notes/fabric.md) | Not yet |
| 3 | [C++ public API](../spec/03-cpp-public-api.md) | Draft | [CLI](../notes/cli.md) | Not yet |
| 4 | [Service model](../spec/04-service-model.md) | Draft | [Services](../notes/service.md) | Not yet |
| 5 | [Wire protocol](../spec/05-wire-protocol.md) | Draft | [Fabric](../notes/fabric.md), [Versioning](../notes/protocol-versioning.md) | Framing + decoder in `pluribusfabric` |
| 6 | [Node/runtime boundary](../spec/06-node-runtime-boundary.md) | Draft | [Node](../notes/node.md), [Runtime](../notes/runtime.md), [Sandbox](../notes/sandbox.md) | Not yet |
| 7 | [Scheduler V1](../spec/07-scheduler-v1.md) | Draft | [Scheduler](../notes/scheduler.md) | `pluribusscheduler` with tests |
| 8 | [Artifact/store protocol](../spec/08-artifact-store-protocol.md) | Draft | [Artifact store](../notes/artifact-store.md), [Packages](../notes/package.md) | Not yet |
| 9 | [Repository/CMake structure](../spec/09-repository-cmake-structure.md) | Draft | [Repository structure](../notes/repository-structure.md) | Initial targets + presets |
| 10 | [Security model](../spec/10-security-model.md) | Draft | [Security](../notes/security.md) | Framing limits only |

## How to use a spec

- Treat **MUST / SHOULD / MAY** as RFC-2119 terms.
- Treat sketches as illustrative unless the spec says otherwise.
- Check the **Open issues** section before relying on a detail.
- Use the decision map to understand why a rule exists.
- When implementation and spec diverge, either fix the code or update the spec deliberately; do not
  leave a silent mismatch.

## Canonical status labels

- **Draft** — design specified but not frozen or fully implemented.
- **Implemented** — reflected in code and tests.
- **Superseded** — retained for history; see the replacement.

## Links

- [Architecture map](architecture.md)
- [Decision map](decisions.md)
- [Open questions](open-questions.md)
- [Implementation map](implementation.md)
