---
type: decision
id: decisions-control-plane
status: accepted
tags: [decision, master, scheduler, control-plane, health]
last_reviewed: 2026-11-25
---

# Control plane, scheduling and health decisions

> Master state, FIFO scheduling, resources, heartbeat, outage behavior and Thor platform targeting.

Each entry records the accepted decision and where it applies. The canonical document remains the
source of context and consequences; expand an entry when the decision is revisited.

## Q-16

**Decision.** Optional resources, unknown is not 1 CPU/0 memory; master defaults, accounting only

**Applies to.** [Scheduler](../architecture/03-control-plane.md#32-scheduler)

---

## Q-17

**Decision.** FIFO; busy compatible nodes queue, absent compatible nodes queue or fail per policy

**Applies to.** [Scheduler](../architecture/03-control-plane.md#32-scheduler)

---

## Q-18

**Decision.** Master-configurable heartbeat; default 1 s / 5 misses

**Applies to.** [Health](../architecture/03-control-plane.md#33-node-health)

---

## Q-19

**Decision.** Nodes keep supervising executions through master outage

**Applies to.** [Failure model](../architecture/03-control-plane.md#34-failure-model)

---

## Q-20

**Decision.** In-memory initially; reconstruct active state and add durability behind storage abstraction

**Applies to.** [Persistence](../architecture/03-control-plane.md#35-master-state--persistence)

---

## Q-25

**Decision.** Initial Thor target is DRIVE OS Linux; validate toolchain, keep core generic

**Applies to.** [Backend](../architecture/04-node-and-runtime.md#43-host-backend-boundary)

---

## Q-29

**Decision.** Static master configuration V1; discovery optional later

**Applies to.** [Fabric](../architecture/05-fabric-and-services.md#53-reliability-semantics)

## Navigation

- [Decision records index](README.md)
- [Decision map](../maps/decisions.md)
- [Architecture map](../maps/architecture.md)
- [Specification map](../maps/specifications.md)
