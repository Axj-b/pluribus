---
type: decision
id: decisions-execution
status: accepted
tags: [decision, execution, core, lifecycle, sandbox]
last_reviewed: 2026-11-25
---

# Execution and core model decisions

> Execution identity, lifecycle, labels, ownership, paths, sandbox retention and resource modeling.

Each entry records the accepted decision and where it applies. The canonical document remains the
source of context and consequences; expand an entry when the decision is revisited.

## Q-10

**Decision.** Node UUID and configurable name; immutable execution identity and separate friendly ID

**Applies to.** [Identifiers](../architecture/02-core-model.md#21-strong-identifiers)

---

## Q-11

**Decision.** Optional opaque labels for correlation; scheduler never interprets them

**Applies to.** [ExecutionSpec](../architecture/02-core-model.md#22-executionspec)

---

## Q-12

**Decision.** Separate timestamped stdout/stderr, bounded configurable history, replay then live stream

**Applies to.** [Node](../architecture/04-node-and-runtime.md#45-process-management-details)

---

## Q-13

**Decision.** Reconcile active executions on reconnect; adopt known, configured orphan policy for stale/unknown

**Applies to.** [Failure model](../architecture/03-control-plane.md#34-failure-model)

---

## Q-21

**Decision.** Execution and client lifetime separate; detach supported

**Applies to.** [Ownership](../architecture/03-control-plane.md#36-client-sessions--ownership)

---

## Q-23

**Decision.** Prefer working-directory-relative paths; env bridges; no V1 filesystem virtualization

**Applies to.** [Runtime](../architecture/04-node-and-runtime.md#42-pluribussim)

---

## Q-26

**Decision.** Master-configurable sandbox retention; store persistent artifacts separately

**Applies to.** [Sandbox](../architecture/04-node-and-runtime.md#44-execution-sandbox)

---

## Q-27

**Decision.** Represent GPU capabilities/resources for future allocation, not complex allocation V1

**Applies to.** [Node](../architecture/04-node-and-runtime.md#45-process-management-details)

## Navigation

- [Decision records index](README.md)
- [Decision map](../maps/decisions.md)
- [Architecture map](../maps/architecture.md)
- [Specification map](../maps/specifications.md)
