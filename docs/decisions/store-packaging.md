---
type: decision
id: decisions-store-packaging
status: accepted
tags: [decision, store, artifacts, packages, abi]
last_reviewed: 2026-11-25
---

# Store, packaging and ABI decisions

> Compatibility, native packaging, executable syntax, artifact caching and transfer strategy.

Each entry records the accepted decision and where it applies. The canonical document remains the
source of context and consequences; expand an entry when the decision is revisited.

## Q-3

**Decision.** Compatibility is Pluribus ABI + CPU architecture, not host OS

**Applies to.** [Packaging](../architecture/06-store-and-packaging.md#65-packages-plp)

---

## Q-4

**Decision.** Native binaries only in V1; revisit runtime mechanism after prototyping

**Applies to.** [Packaging](../architecture/06-store-and-packaging.md#65-packages-plp)

---

## Q-5

**Decision.** Explicit package, local file, artifact and version syntax; no node PATH lookup

**Applies to.** [Packaging](../architecture/06-store-and-packaging.md#65-packages-plp)

---

## Q-36

**Decision.** Configurable LRU cache, node-selected location, pin active artifacts

**Applies to.** [Cache](../architecture/06-store-and-packaging.md#62-local-cache)

---

## Q-37

**Decision.** Peer-to-peer artifact transfer long-term; V1 may relay via master

**Applies to.** [Transfer](../architecture/06-store-and-packaging.md#63-transfer)

## Navigation

- [Decision records index](README.md)
- [Decision map](../maps/decisions.md)
- [Architecture map](../maps/architecture.md)
- [Specification map](../maps/specifications.md)
