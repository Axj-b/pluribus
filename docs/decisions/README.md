---
type: index
id: decisions
status: active
tags: [decision, map]
last_reviewed: 2026-11-25
---

# Decision records

> The 47 accepted decisions are grouped into six thematic files. Each section keeps its stable
> `Q-N` ID, the decision text and a link to the canonical document where it applies.

The canonical document remains the source of context and consequences. Expand a decision section
when it is revisited or superseded.

## Themed files

| File | Scope | Decisions |
|---|---|---|
| [scope.md](scope.md) | Product scope, naming, documentation order and delivery approach. | Q-1, Q-2, Q-7, Q-8, Q-9, Q-45, Q-46 |
| [execution.md](execution.md) | Execution identity, lifecycle, labels, ownership, paths, sandbox retention and resource modeling. | Q-10, Q-11, Q-12, Q-13, Q-21, Q-23, Q-26, Q-27 |
| [control-plane.md](control-plane.md) | Master state, FIFO scheduling, resources, heartbeat, outage behavior and Thor platform targeting. | Q-16, Q-17, Q-18, Q-19, Q-20, Q-25, Q-29 |
| [fabric-services.md](fabric-services.md) | Wire codec, idempotency, service naming, provider resolution, stream semantics and networking boundaries. | Q-15, Q-28, Q-30, Q-31, Q-32, Q-33, Q-34, Q-35, Q-47 |
| [store-packaging.md](store-packaging.md) | Compatibility, native packaging, executable syntax, artifact caching and transfer strategy. | Q-3, Q-4, Q-5, Q-36, Q-37 |
| [engineering.md](engineering.md) | Public API, configuration, toolchains, dependencies, concurrency, security baseline, logging, build enforcement and CLI behavior. | Q-6, Q-14, Q-22, Q-24, Q-38, Q-39, Q-40, Q-41, Q-42, Q-43, Q-44 |

## Full index

| ID | Decision | Applies to | Record |
|---|---|---|---|
| Q-1 | RadarScape first consumer, then distributed build/test and experimentation; platform domain-agnostic | [Overview](../architecture/01-overview.md#12-goals) | [Q-1](scope.md#q-1) |
| Q-2 | Initially 2–10 nodes and well below 1000 concurrent executions; do not preclude growth | [Roadmap](../architecture/08-roadmap.md#project-scope) | [Q-2](scope.md#q-2) |
| Q-3 | Compatibility is Pluribus ABI + CPU architecture, not host OS | [Packaging](../architecture/06-store-and-packaging.md#65-packages-plp) | [Q-3](store-packaging.md#q-3) |
| Q-4 | Native binaries only in V1; revisit runtime mechanism after prototyping | [Packaging](../architecture/06-store-and-packaging.md#65-packages-plp) | [Q-4](store-packaging.md#q-4) |
| Q-5 | Explicit package, local file, artifact and version syntax; no node PATH lookup | [Packaging](../architecture/06-store-and-packaging.md#65-packages-plp) | [Q-5](store-packaging.md#q-5) |
| Q-6 | C++23 client API, no stable C ABI initially; avoid implementation-specific types | [Public API](../architecture/05-fabric-and-services.md#56-public-api-surface-v1) | [Q-6](engineering.md#q-6) |
| Q-7 | Start from scratch; no RadarScape coupling | [Overview](../architecture/01-overview.md#12-goals) | [Q-7](scope.md#q-7) |
| Q-8 | Pluribus, `pluribus::`, `plu`; evaluate a standard free attribution license before publication | [Roadmap](../architecture/08-roadmap.md#project-scope) | [Q-8](scope.md#q-8) |
| Q-9 | Timeline/team undecided; milestone-driven | [Roadmap](../architecture/08-roadmap.md#milestones) | [Q-9](scope.md#q-9) |
| Q-10 | Node UUID and configurable name; immutable execution identity and separate friendly ID | [Identifiers](../architecture/02-core-model.md#21-strong-identifiers) | [Q-10](execution.md#q-10) |
| Q-11 | Optional opaque labels for correlation; scheduler never interprets them | [ExecutionSpec](../architecture/02-core-model.md#22-executionspec) | [Q-11](execution.md#q-11) |
| Q-12 | Separate timestamped stdout/stderr, bounded configurable history, replay then live stream | [Node](../architecture/04-node-and-runtime.md#45-process-management-details) | [Q-12](execution.md#q-12) |
| Q-13 | Reconcile active executions on reconnect; adopt known, configured orphan policy for stale/unknown | [Failure model](../architecture/03-control-plane.md#34-failure-model) | [Q-13](execution.md#q-13) |
| Q-14 | Auto-detect capabilities plus configuration; host OS not a scheduling capability | [NodeDescriptor](../architecture/02-core-model.md#25-nodedescriptor) | [Q-14](engineering.md#q-14) |
| Q-15 | Benchmark FlatBuffers, Cap'n Proto and Protobuf before choosing | [Technology](../architecture/07-cross-cutting.md#71-technology-choices-v1) | [Q-15](fabric-services.md#q-15) |
| Q-16 | Optional resources, unknown is not 1 CPU/0 memory; master defaults, accounting only | [Scheduler](../architecture/03-control-plane.md#32-scheduler) | [Q-16](control-plane.md#q-16) |
| Q-17 | FIFO; busy compatible nodes queue, absent compatible nodes queue or fail per policy | [Scheduler](../architecture/03-control-plane.md#32-scheduler) | [Q-17](control-plane.md#q-17) |
| Q-18 | Master-configurable heartbeat; default 1 s / 5 misses | [Health](../architecture/03-control-plane.md#33-node-health) | [Q-18](control-plane.md#q-18) |
| Q-19 | Nodes keep supervising executions through master outage | [Failure model](../architecture/03-control-plane.md#34-failure-model) | [Q-19](control-plane.md#q-19) |
| Q-20 | In-memory initially; reconstruct active state and add durability behind storage abstraction | [Persistence](../architecture/03-control-plane.md#35-master-state--persistence) | [Q-20](control-plane.md#q-20) |
| Q-21 | Execution and client lifetime separate; detach supported | [Ownership](../architecture/03-control-plane.md#36-client-sessions--ownership) | [Q-21](execution.md#q-21) |
| Q-22 | TOML configuration; foreground V1 | [Node](../architecture/04-node-and-runtime.md#41-pluribusnode) | [Q-22](engineering.md#q-22) |
| Q-23 | Prefer working-directory-relative paths; env bridges; no V1 filesystem virtualization | [Runtime](../architecture/04-node-and-runtime.md#42-pluribussim) | [Q-23](execution.md#q-23) |
| Q-24 | C++ service API and local language-neutral endpoint; port forwarding later | [Runtime](../architecture/04-node-and-runtime.md#42-pluribussim) | [Q-24](engineering.md#q-24) |
| Q-25 | Initial Thor target is DRIVE OS Linux; validate toolchain, keep core generic | [Backend](../architecture/04-node-and-runtime.md#43-host-backend-boundary) | [Q-25](control-plane.md#q-25) |
| Q-26 | Master-configurable sandbox retention; store persistent artifacts separately | [Sandbox](../architecture/04-node-and-runtime.md#44-execution-sandbox) | [Q-26](execution.md#q-26) |
| Q-27 | Represent GPU capabilities/resources for future allocation, not complex allocation V1 | [Node](../architecture/04-node-and-runtime.md#45-process-management-details) | [Q-27](execution.md#q-27) |
| Q-28 | Idempotent submit with client request ID, sequence numbers and 1 MiB control limit | [Fabric](../architecture/05-fabric-and-services.md#53-reliability-semantics) | [Q-28](fabric-services.md#q-28) |
| Q-29 | Static master configuration V1; discovery optional later | [Fabric](../architecture/05-fabric-and-services.md#53-reliability-semantics) | [Q-29](control-plane.md#q-29) |
| Q-30 | Hierarchical logical service names distinct from provider instance | [Services](../architecture/05-fabric-and-services.md#54-service-model) | [Q-30](fabric-services.md#q-30) |
| Q-31 | Protocol permits multiple providers; V1 resolves one | [Services](../architecture/05-fabric-and-services.md#54-service-model) | [Q-31](fabric-services.md#q-31) |
| Q-32 | Reliable ordered byte stream minimum, extensible to future modes | [Services](../architecture/05-fabric-and-services.md#54-service-model) | [Q-32](fabric-services.md#q-32) |
| Q-33 | Explicit READY before resolution; wait/timeout supported | [Registry](../architecture/05-fabric-and-services.md#registry-operations) | [Q-33](fabric-services.md#q-33) |
| Q-34 | Normal networking/hardware remain accessible; no general virtual network stack | [Networking](../architecture/05-fabric-and-services.md#55-host-independent-networking) | [Q-34](fabric-services.md#q-34) |
| Q-35 | C++23 library plus documented language-neutral protocol; no C++ layout on wire | [Public API](../architecture/05-fabric-and-services.md#56-public-api-surface-v1) | [Q-35](fabric-services.md#q-35) |
| Q-36 | Configurable LRU cache, node-selected location, pin active artifacts | [Cache](../architecture/06-store-and-packaging.md#62-local-cache) | [Q-36](store-packaging.md#q-36) |
| Q-37 | Peer-to-peer artifact transfer long-term; V1 may relay via master | [Transfer](../architecture/06-store-and-packaging.md#63-transfer) | [Q-37](store-packaging.md#q-37) |
| Q-38 | C++23 toolchain matrix; pin versions in CI, validate Thor | [Technology](../architecture/07-cross-cutting.md#71-technology-choices-v1) | [Q-38](engineering.md#q-38) |
| Q-39 | Small maintained dependencies, reproducible CMake package strategy, benchmark stack | [Technology](../architecture/07-cross-cutting.md#71-technology-choices-v1) | [Q-39](engineering.md#q-39) |
| Q-40 | Async event-driven I/O, coroutines acceptable, small worker pool | [Technology](../architecture/07-cross-cutting.md#71-technology-choices-v1) | [Q-40](engineering.md#q-40) |
| Q-41 | Trusted-network V1 with join token, explicit bind, validation and size limits | [Security](../architecture/07-cross-cutting.md#72-security-v1) | [Q-41](engineering.md#q-41) |
| Q-42 | Structured logs and human-readable interactive console | [Observability](../architecture/07-cross-cutting.md#73-observability) | [Q-42](engineering.md#q-42) |
| Q-43 | Enforce component boundaries using CMake and CI checks | [Dependencies](../architecture/08-roadmap.md#dependency-direction) | [Q-43](engineering.md#q-43) |
| Q-44 | `--json`, explicit config precedence, foreground wait/stream/exit code and `--detach` | [CLI](../architecture/08-roadmap.md#cli-plu) | [Q-44](engineering.md#q-44) |
| Q-45 | Uniform distributed environment, not Linux simulator; what/where/how invariant | [Overview](../architecture/01-overview.md#11-vision) | [Q-45](scope.md#q-45) |
| Q-46 | Ordered specifications, then first end-to-end remote hello before expansion | [Roadmap](../architecture/08-roadmap.md#documentation-priorities) | [Q-46](scope.md#q-46) |
| Q-47 | Use Protocol Buffers for V1 control payloads after the codec benchmark | [Wire protocol](../spec/05-wire-protocol.md#serialization-benchmark-and-selection) | [Q-47](fabric-services.md#q-47) |

## See also

- [Decision map](../maps/decisions.md) — thematic navigation
- [Knowledge base guide](../knowledge-base.md) — documentation workflow
- [Open questions](../questions/README.md) — unresolved issues
