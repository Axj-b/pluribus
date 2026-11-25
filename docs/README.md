# Pluribus Documentation

Source idea: [`../IDEA.md`](../IDEA.md), retained as the original idea. These documents record the
current decisions. The answered questionnaire is a local, not committed source note.
Status: **draft architecture; implementation and ABI/runtime validation pending.**

## Architecture

| # | Document | Content |
|---|----------|---------|
| 1 | [architecture/01-overview.md](architecture/01-overview.md) | Vision, principles, glossary, layering, invariants |
| 2 | [architecture/02-core-model.md](architecture/02-core-model.md) | Identifiers, ExecutionSpec, handle, state machine, node descriptor |
| 3 | [architecture/03-control-plane.md](architecture/03-control-plane.md) | Master, scheduler, registries, failure model |
| 4 | [architecture/04-node-and-runtime.md](architecture/04-node-and-runtime.md) | Node agent, pluribussim, host backend, sandbox |
| 5 | [architecture/05-fabric-and-services.md](architecture/05-fabric-and-services.md) | Transport, protocol, service model and routing |
| 6 | [architecture/06-store-and-packaging.md](architecture/06-store-and-packaging.md) | Artifacts, caching, packages |
| 7 | [architecture/07-cross-cutting.md](architecture/07-cross-cutting.md) | Security, observability, performance, testing, tech stack |
| 8 | [architecture/08-roadmap.md](architecture/08-roadmap.md) | Repository layout, milestones, prototypes, non-goals |

## Specifications in priority order

Decision (Q-46): Develop these planned specifications in order, then implement the first end-to-end
`plu run hello` before expanding into domain workloads or advanced platform features.

1. [Execution model and state machine](spec/01-execution-model.md)
2. [Node/master protocol](spec/02-node-master-protocol.md)
3. [C++23 public API](spec/03-cpp-public-api.md)
4. [Service model and lifecycle](spec/04-service-model.md)
5. [Wire protocol and reconnect/idempotency](spec/05-wire-protocol.md)
6. [Node/runtime boundary](spec/06-node-runtime-boundary.md)
7. [Scheduler V1](spec/07-scheduler-v1.md)
8. [Artifact/store protocol](spec/08-artifact-store-protocol.md)
9. [Repository/CMake structure](spec/09-repository-cmake-structure.md)
10. [Security model](spec/10-security-model.md)

## Decisions

| Question | Decision | Applied in |
|----------|----------|------------|
| Q-1 | RadarScape first consumer, then distributed build/test and experimentation; platform domain-agnostic | [Overview](architecture/01-overview.md#12-goals) |
| Q-2 | Initially 2–10 nodes and well below 1000 concurrent executions; do not preclude growth | [Roadmap](architecture/08-roadmap.md#project-scope) |
| Q-3 | Compatibility is Pluribus ABI + CPU architecture, not host OS | [Packaging](architecture/06-store-and-packaging.md#65-packages-plp) |
| Q-4 | Native binaries only in V1; revisit runtime mechanism after prototyping | [Packaging](architecture/06-store-and-packaging.md#65-packages-plp) |
| Q-5 | Explicit package, local file, artifact and version syntax; no node PATH lookup | [Packaging](architecture/06-store-and-packaging.md#65-packages-plp) |
| Q-6 | C++23 client API, no stable C ABI initially; avoid implementation-specific types | [Public API](architecture/05-fabric-and-services.md#56-public-api-surface-v1) |
| Q-7 | Start from scratch; no RadarScape coupling | [Overview](architecture/01-overview.md#12-goals) |
| Q-8 | Pluribus, `pluribus::`, `plu`; evaluate a standard free attribution license before publication | [Roadmap](architecture/08-roadmap.md#project-scope) |
| Q-9 | Timeline/team undecided; milestone-driven | [Roadmap](architecture/08-roadmap.md#milestones) |
| Q-10 | Node UUID and configurable name; immutable execution identity and separate friendly ID | [Identifiers](architecture/02-core-model.md#21-strong-identifiers) |
| Q-11 | Optional opaque labels for correlation; scheduler never interprets them | [ExecutionSpec](architecture/02-core-model.md#22-executionspec) |
| Q-12 | Separate timestamped stdout/stderr, bounded configurable history, replay then live stream | [Node](architecture/04-node-and-runtime.md#45-process-management-details) |
| Q-13 | Reconcile active executions on reconnect; adopt known, configured orphan policy for stale/unknown | [Failure model](architecture/03-control-plane.md#34-failure-model) |
| Q-14 | Auto-detect capabilities plus configuration; host OS not a scheduling capability | [NodeDescriptor](architecture/02-core-model.md#25-nodedescriptor) |
| Q-15 | Benchmark FlatBuffers, Cap'n Proto and Protobuf before choosing | [Technology](architecture/07-cross-cutting.md#71-technology-choices-v1) |
| Q-16 | Optional resources, unknown is not 1 CPU/0 memory; master defaults, accounting only | [Scheduler](architecture/03-control-plane.md#32-scheduler) |
| Q-17 | FIFO; busy compatible nodes queue, absent compatible nodes queue or fail per policy | [Scheduler](architecture/03-control-plane.md#32-scheduler) |
| Q-18 | Master-configurable heartbeat; default 1 s / 5 misses | [Health](architecture/03-control-plane.md#33-node-health) |
| Q-19 | Nodes keep supervising executions through master outage | [Failure model](architecture/03-control-plane.md#34-failure-model) |
| Q-20 | In-memory initially; reconstruct active state and add durability behind storage abstraction | [Persistence](architecture/03-control-plane.md#35-master-state--persistence) |
| Q-21 | Execution and client lifetime separate; detach supported | [Ownership](architecture/03-control-plane.md#36-client-sessions--ownership) |
| Q-22 | TOML configuration; foreground V1 | [Node](architecture/04-node-and-runtime.md#41-pluribusnode) |
| Q-23 | Prefer working-directory-relative paths; env bridges; no V1 filesystem virtualization | [Runtime](architecture/04-node-and-runtime.md#42-pluribussim) |
| Q-24 | C++ service API and local language-neutral endpoint; port forwarding later | [Runtime](architecture/04-node-and-runtime.md#42-pluribussim) |
| Q-25 | Initial Thor target is DRIVE OS Linux; validate toolchain, keep core generic | [Backend](architecture/04-node-and-runtime.md#43-host-backend-boundary) |
| Q-26 | Master-configurable sandbox retention; store persistent artifacts separately | [Sandbox](architecture/04-node-and-runtime.md#44-execution-sandbox) |
| Q-27 | Represent GPU capabilities/resources for future allocation, not complex allocation V1 | [Node](architecture/04-node-and-runtime.md#45-process-management-details) |
| Q-28 | Idempotent submit with client request ID, sequence numbers and 1 MiB control limit | [Fabric](architecture/05-fabric-and-services.md#53-reliability-semantics) |
| Q-29 | Static master configuration V1; discovery optional later | [Fabric](architecture/05-fabric-and-services.md#53-reliability-semantics) |
| Q-30 | Hierarchical logical service names distinct from provider instance | [Services](architecture/05-fabric-and-services.md#54-service-model) |
| Q-31 | Protocol permits multiple providers; V1 resolves one | [Services](architecture/05-fabric-and-services.md#54-service-model) |
| Q-32 | Reliable ordered byte stream minimum, extensible to future modes | [Services](architecture/05-fabric-and-services.md#54-service-model) |
| Q-33 | Explicit READY before resolution; wait/timeout supported | [Registry](architecture/05-fabric-and-services.md#registry-operations) |
| Q-34 | Normal networking/hardware remain accessible; no general virtual network stack | [Networking](architecture/05-fabric-and-services.md#55-host-independent-networking) |
| Q-35 | C++23 library plus documented language-neutral protocol; no C++ layout on wire | [Public API](architecture/05-fabric-and-services.md#56-public-api-surface-v1) |
| Q-36 | Configurable LRU cache, node-selected location, pin active artifacts | [Cache](architecture/06-store-and-packaging.md#62-local-cache) |
| Q-37 | Peer-to-peer artifact transfer long-term; V1 may relay via master | [Transfer](architecture/06-store-and-packaging.md#63-transfer) |
| Q-38 | C++23 toolchain matrix; pin versions in CI, validate Thor | [Technology](architecture/07-cross-cutting.md#71-technology-choices-v1) |
| Q-39 | Small maintained dependencies, reproducible CMake package strategy, benchmark stack | [Technology](architecture/07-cross-cutting.md#71-technology-choices-v1) |
| Q-40 | Async event-driven I/O, coroutines acceptable, small worker pool | [Technology](architecture/07-cross-cutting.md#71-technology-choices-v1) |
| Q-41 | Trusted-network V1 with join token, explicit bind, validation and size limits | [Security](architecture/07-cross-cutting.md#72-security-v1) |
| Q-42 | Structured logs and human-readable interactive console | [Observability](architecture/07-cross-cutting.md#73-observability) |
| Q-43 | Enforce component boundaries using CMake and CI checks | [Dependencies](architecture/08-roadmap.md#dependency-direction) |
| Q-44 | `--json`, explicit config precedence, foreground wait/stream/exit code and `--detach` | [CLI](architecture/08-roadmap.md#cli-plu) |
| Q-45 | Uniform distributed environment, not Linux simulator; what/where/how invariant | [Overview](architecture/01-overview.md#11-vision) |
| Q-46 | Ordered specifications, then first end-to-end remote hello before expansion | [Roadmap](architecture/08-roadmap.md#documentation-priorities) |

Conventions:
- **MUST / SHOULD / MAY** are used in the RFC-2119 sense.
- Q-n identifies an answered design decision in the local, not committed questionnaire.
- Component names: `pluribuscore`, `pluribusmaster`, `pluribusnode`, `pluribussim`, `pluribusfabric`, `pluribusstore`, CLI component `plucli` (binary `plu`).
