---
type: decision
id: decisions-fabric-services
status: accepted
tags: [decision, fabric, services, protocol, wire]
last_reviewed: 2026-11-25
---

# Fabric, protocol and service decisions

> Wire codec, idempotency, service naming, provider resolution, stream semantics and networking boundaries.

Each entry records the accepted decision and where it applies. The canonical document remains the
source of context and consequences; expand an entry when the decision is revisited.

## Q-15

**Decision.** Benchmark FlatBuffers, Cap'n Proto and Protobuf before choosing

**Applies to.** [Technology](../architecture/07-cross-cutting.md#71-technology-choices-v1)

---

## Q-28

**Decision.** Idempotent submit with client request ID, sequence numbers and 1 MiB control limit

**Applies to.** [Fabric](../architecture/05-fabric-and-services.md#53-reliability-semantics)

---

## Q-30

**Decision.** Hierarchical logical service names distinct from provider instance

**Applies to.** [Services](../architecture/05-fabric-and-services.md#54-service-model)

---

## Q-31

**Decision.** Protocol permits multiple providers; V1 resolves one

**Applies to.** [Services](../architecture/05-fabric-and-services.md#54-service-model)

---

## Q-32

**Decision.** Reliable ordered byte stream minimum, extensible to future modes

**Applies to.** [Services](../architecture/05-fabric-and-services.md#54-service-model)

---

## Q-33

**Decision.** Explicit READY before resolution; wait/timeout supported

**Applies to.** [Registry](../architecture/05-fabric-and-services.md#registry-operations)

---

## Q-34

**Decision.** Normal networking/hardware remain accessible; no general virtual network stack

**Applies to.** [Networking](../architecture/05-fabric-and-services.md#55-host-independent-networking)

---

## Q-35

**Decision.** C++23 library plus documented language-neutral protocol; no C++ layout on wire

**Applies to.** [Public API](../architecture/05-fabric-and-services.md#56-public-api-surface-v1)

## Navigation

- [Decision records index](README.md)
- [Decision map](../maps/decisions.md)
- [Architecture map](../maps/architecture.md)
- [Specification map](../maps/specifications.md)
