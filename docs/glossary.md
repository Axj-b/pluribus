---
type: guide
id: glossary
status: active
tags: [guide, glossary, terminology]
aliases: ["Glossary", "Terms"]
last_reviewed: 2026-11-25
---

# Glossary

> One place to disambiguate the words that appear across the architecture, specs and code.

| Term | Meaning | Canonical source |
|---|---|---|
| ABI (Pluribus ABI) | Runtime/executable compatibility contract; combined with CPU architecture for scheduling. Not the wire protocol or host OS. | [Runtime boundary](spec/06-node-runtime-boundary.md#abi-identity-and-configuration), [Packaging](architecture/06-store-and-packaging.md#65-packages-plp) |
| ADR | Architecture decision record. Here: a short note in [`decisions/`](decisions/README.md) with status, decision and links. | [Decision records](decisions/README.md) |
| Artifact | Immutable content-addressed blob, identified by `sha256:<digest>`. | [Artifact store](spec/08-artifact-store-protocol.md) |
| Capability | Qualitative node feature with an extensible name, e.g. `gpu.cuda` or `target.thor`. Not a quantitative resource. | [Node descriptor](architecture/02-core-model.md#25-nodedescriptor) |
| Control plane | Small messages: registration, heartbeat, submit, assignment, state, services, terminate. | [Fabric](architecture/05-fabric-and-services.md) |
| Data plane | Potentially large streams: artifacts, logs, application/service bytes. Independent backpressure from control. | [Wire protocol](spec/05-wire-protocol.md) |
| Execution | One run of a native Pluribus-compatible executable. The only scheduling primitive; no BuildJob/TestJob/RadarJob in core. | [Execution model](spec/01-execution-model.md) |
| ExecutionSpec | Immutable description of an execution: executable, arguments, inputs, outputs, lifetime, services, constraints and labels. | [Execution model](spec/01-execution-model.md) |
| ExecutionHandle | Client-side reference for querying, waiting, logs, stopping and outputs. Destroying it does not terminate the execution. | [Public API](spec/03-cpp-public-api.md) |
| Fabric | Communication substrate between clients, master and nodes. | [Fabric](architecture/05-fabric-and-services.md) |
| Map of content (MOC) | Curated entry point into a domain, linking notes, decisions, questions and canonical docs. | [Maps](maps/README.md) |
| Master | Authoritative control plane for one Pluribus machine: registry, queue, scheduling, ownership, services, artifact index. | [Control plane](architecture/03-control-plane.md) |
| Node | Per-host agent that registers, heartbeats, materializes inputs, launches/supervises executions, streams logs and exposes services. | [Node](architecture/04-node-and-runtime.md) |
| Package (`.plp`) | Immutable bundle with a manifest and per-ABI/architecture native entries and resources. | [Packaging](spec/08-artifact-store-protocol.md#package-model) |
| Pluribus machine | One master plus N nodes presented as one logical machine. | [Overview](architecture/01-overview.md) |
| pluribussim | Uniform runtime/environment layer and host-backend boundary. | [Runtime boundary](spec/06-node-runtime-boundary.md) |
| Provider | A concrete service instance exported by an execution, with readiness and routing metadata. | [Service model](spec/04-service-model.md) |
| Resource | Quantitative CPU or memory amount. Absent means unknown, not zero. | [Scheduler](spec/07-scheduler-v1.md) |
| Sandbox | Per-execution logical workspace (`input/`, `work/`, `output/`, `tmp/`). Operational, not a security boundary in V1. | [Sandbox](architecture/04-node-and-runtime.md#44-execution-sandbox) |
| Scheduler | Pure function that maps a request plus node snapshots to a `NodeId` (or a temporary/fundamental failure). | [Scheduler V1](spec/07-scheduler-v1.md) |
| Second brain | The linked docs layer described in the [knowledge base guide](knowledge-base.md). | [Knowledge base](knowledge-base.md) |
| Service | Logical, location-independent name such as `radarscape/sar/output`; resolved to a ready provider. | [Service model](spec/04-service-model.md) |
| Snapshot (workspace) | Immutable tree of paths mapped to blob hashes, proposed for content-addressed workspace caching. | [Workspace cache idea](../todo/content-addressed-workspace-cache.md) |
| Wire protocol | Language-neutral framing, handshake, sequencing and compatibility contract. Distinct from ABI and package format. | [Wire protocol](spec/05-wire-protocol.md) |
