---
type: decision
id: decisions-engineering
status: accepted
tags: [decision, engineering, api, cmake, testing, security, cli]
last_reviewed: 2026-11-25
---

# Engineering, API and operations decisions

> Public API, configuration, toolchains, dependencies, concurrency, security baseline, logging, build enforcement and CLI behavior.

Each entry records the accepted decision and where it applies. The canonical document remains the
source of context and consequences; expand an entry when the decision is revisited.

## Q-6

**Decision.** C++23 client API, no stable C ABI initially; avoid implementation-specific types

**Applies to.** [Public API](../architecture/05-fabric-and-services.md#56-public-api-surface-v1)

---

## Q-14

**Decision.** Auto-detect capabilities plus configuration; host OS not a scheduling capability

**Applies to.** [NodeDescriptor](../architecture/02-core-model.md#25-nodedescriptor)

---

## Q-22

**Decision.** TOML configuration; foreground V1

**Applies to.** [Node](../architecture/04-node-and-runtime.md#41-pluribusnode)

---

## Q-24

**Decision.** C++ service API and local language-neutral endpoint; port forwarding later

**Applies to.** [Runtime](../architecture/04-node-and-runtime.md#42-pluribussim)

---

## Q-38

**Decision.** C++23 toolchain matrix; pin versions in CI, validate Thor

**Applies to.** [Technology](../architecture/07-cross-cutting.md#71-technology-choices-v1)

---

## Q-39

**Decision.** Small maintained dependencies, reproducible CMake package strategy, benchmark stack

**Applies to.** [Technology](../architecture/07-cross-cutting.md#71-technology-choices-v1)

---

## Q-40

**Decision.** Async event-driven I/O, coroutines acceptable, small worker pool

**Applies to.** [Technology](../architecture/07-cross-cutting.md#71-technology-choices-v1)

---

## Q-41

**Decision.** Trusted-network V1 with join token, explicit bind, validation and size limits

**Applies to.** [Security](../architecture/07-cross-cutting.md#72-security-v1)

---

## Q-42

**Decision.** Structured logs and human-readable interactive console

**Applies to.** [Observability](../architecture/07-cross-cutting.md#73-observability)

---

## Q-43

**Decision.** Enforce component boundaries using CMake and CI checks

**Applies to.** [Dependencies](../architecture/08-roadmap.md#dependency-direction)

---

## Q-44

**Decision.** `--json`, explicit config precedence, foreground wait/stream/exit code and `--detach`

**Applies to.** [CLI](../architecture/08-roadmap.md#cli-plu)

## Navigation

- [Decision records index](README.md)
- [Decision map](../maps/decisions.md)
- [Architecture map](../maps/architecture.md)
- [Specification map](../maps/specifications.md)
