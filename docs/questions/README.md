---
type: index
id: open-questions
status: active
tags: [question, register]
aliases: ["Open questions", "Open issues"]
last_reviewed: 2026-11-25
---

# Open questions register

> The canonical register of unresolved design and implementation questions. Status changes here;
> the theme view is in the [open-question map](../maps/open-questions.md).

## OI-01 ExecutionId representation

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues), [Core model](../architecture/02-core-model.md#21-strong-identifiers)
- **Why:** identity, friendly short IDs and lookup ambiguity affect master storage and CLI behavior.
- **Next action:** choose the on-wire representation and short-ID collision policy with the first master implementation.
- **Related:** [Q-10](../decisions/execution.md#q-10), [Execution lifecycle](../notes/execution-lifecycle.md)

## OI-02 Owner identity and lifetime

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues), [Public API](../spec/03-cpp-public-api.md#open-issues)
- **Why:** detach and owner-lifetime coupling need an identity independent of the socket.
- **Next action:** define client identity and the owner-lifetime mechanism before implementing `--detach`.
- **Related:** [Q-21](../decisions/execution.md#q-21), [Master](../notes/master.md)

## OI-03 Orphan policy and retention

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues), [Runtime boundary](../spec/06-node-runtime-boundary.md#disconnect-reconciliation-and-services)
- **Why:** unknown/stale processes must not be silently killed or adopted; log and outcome retention is finite.
- **Next action:** specify configurable orphan actions and retention durations with the reconciliation implementation.
- **Related:** [Q-13](../decisions/execution.md#q-13), [Node](../notes/node.md)

## OI-04 Portable native ABI

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues), [Runtime boundary](../spec/06-node-runtime-boundary.md#open-issues), [Store](../spec/08-artifact-store-protocol.md#open-issues)
- **Why:** ABI + architecture keys are a contract, not proof that a native binary runs across host OSes.
- **Next action:** prototype the loader/runtime contract and validate packaging on Windows, Linux and DRIVE OS.
- **Related:** [Q-3](../decisions/store-packaging.md#q-3), [Q-4](../decisions/store-packaging.md#q-4), [Package](../notes/package.md)

## OI-05 Exit status and output collection

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues)
- **Why:** portable exit/signal representation and collection failures affect the final execution outcome.
- **Next action:** define portable outcome fields and whether collection failure changes `Completed` to `Failed`.
- **Related:** [Execution lifecycle](../notes/execution-lifecycle.md), [Artifact store](../notes/artifact-store.md)

## OI-06 Control message schema and type registry

- **Status:** open
- **Source:** [Node/master protocol](../spec/02-node-master-protocol.md#open-issues), [Wire protocol](../spec/05-wire-protocol.md#open-issues)
- **Why:** framing is implemented, but payload fields, numeric type IDs, flags and bootstrap encoding are not frozen.
- **Next action:** select a codec (OI-14), define versioned schemas and reserve numeric identifiers.
- **Related:** [Fabric](../notes/fabric.md), [Wire protocol](../spec/05-wire-protocol.md)

## OI-07 Endpoint tickets and route authorization

- **Status:** open
- **Source:** [Node/master protocol](../spec/02-node-master-protocol.md#open-issues), [Service model](../spec/04-service-model.md#open-issues)
- **Why:** resolved services need short-lived routing capabilities without exposing durable host addresses.
- **Next action:** design ticket format, lifetime, binding and failure semantics with service routing.
- **Related:** [Services](../notes/service.md), [Q-30](../decisions/fabric-services.md#q-30)

## OI-08 Authentication and client identity

- **Status:** open
- **Source:** [Node/master protocol](../spec/02-node-master-protocol.md#open-issues), [Wire protocol](../spec/05-wire-protocol.md#open-issues), [Security](../spec/10-security-model.md#open-issues)
- **Why:** a join token is not encryption, client identity, or replay protection.
- **Next action:** define client authorization and token provisioning/rotation; keep TLS as the later hardening path.
- **Related:** [Q-41](../decisions/engineering.md#q-41), [Security](../notes/security.md)

## OI-09 Request deduplication and durable master state

- **Status:** open
- **Source:** [Node/master protocol](../spec/02-node-master-protocol.md#open-issues), [Wire protocol](../spec/05-wire-protocol.md#open-issues)
- **Why:** retry safety depends on retained `(client, request_id)` mappings and canonical fingerprints.
- **Next action:** define retention, fingerprint canonicalization and the persistence boundary before claiming exactly-once submission.
- **Related:** [Q-20](../decisions/control-plane.md#q-20), [Q-28](../decisions/fabric-services.md#q-28), [Master](../notes/master.md)

## OI-10 Public API ownership and async model

- **Status:** open
- **Source:** [Public API](../spec/03-cpp-public-api.md#open-issues)
- **Why:** handles, async cancellation, executor ownership and thread safety shape every client.
- **Next action:** finalize names, construction/validation, handle ownership and async rules before publishing the API.
- **Related:** [Q-6](../decisions/engineering.md#q-6), [CLI](../notes/cli.md)

## OI-11 Log cursor and stream semantics

- **Status:** open
- **Source:** [Public API](../spec/03-cpp-public-api.md#open-issues)
- **Why:** replay gaps, byte buffers, shutdown and error representation must be explicit for consumers.
- **Next action:** define log cursors, gap reporting, stream shutdown and error categories with the node log implementation.
- **Related:** [Q-12](../decisions/execution.md#q-12), [Node](../notes/node.md)

## OI-12 Service-name grammar

- **Status:** open
- **Source:** [Service model](../spec/04-service-model.md#open-issues)
- **Why:** interoperability requires one normalization and allowed-character set.
- **Next action:** freeze the grammar before service registration crosses implementations.
- **Related:** [Services](../notes/service.md), [Q-30](../decisions/fabric-services.md#q-30)

## OI-13 Duplicate provider and readiness policy

- **Status:** open
- **Source:** [Service model](../spec/04-service-model.md#open-issues)
- **Why:** multiple providers are permitted, but V1 resolves one and must reject or hide conflicts deterministically.
- **Next action:** choose the duplicate policy, readiness revalidation, drain and health semantics.
- **Related:** [Q-31](../decisions/fabric-services.md#q-31), [Q-33](../decisions/fabric-services.md#q-33), [Services](../notes/service.md)

## OI-14 Serializer selection

- **Status:** open
- **Source:** [Wire protocol](../spec/05-wire-protocol.md#serialization-benchmark-and-selection), [Q-15](../decisions/fabric-services.md#q-15)
- **Why:** the control codec affects latency, allocations, schema evolution and tooling.
- **Next action:** run the documented benchmark on representative messages and record the decision before freezing payloads.
- **Related:** [Fabric](../notes/fabric.md), [Protocol versioning](../notes/protocol-versioning.md)

## OI-15 Data-plane framing and limits

- **Status:** open
- **Source:** [Wire protocol](../spec/05-wire-protocol.md#open-issues), [Store](../spec/08-artifact-store-protocol.md#open-issues), [Security](../spec/10-security-model.md#open-issues)
- **Why:** bulk traffic must not block control, and object/chunk limits must bound allocation.
- **Next action:** define data-purpose tickets, frame/chunk formats, object limits and concurrent-transfer policy.
- **Related:** [Artifact store](../notes/artifact-store.md), [Fabric](../notes/fabric.md)

## OI-16 Wire compatibility and conformance

- **Status:** open
- **Source:** [Wire protocol](../spec/05-wire-protocol.md#compatibility-and-stable-contract)
- **Why:** schema evolution and unknown-field behavior must be tested, not assumed.
- **Next action:** create conformance fixtures and old/new peer tests before declaring the wire stable.
- **Related:** [Protocol versioning](../notes/protocol-versioning.md)

## OI-17 Host backend signatures and environment

- **Status:** open
- **Source:** [Runtime boundary](../spec/06-node-runtime-boundary.md#open-issues)
- **Why:** backend handles, async integration, log gaps, retention and the exact `PLURIBUS_*` bridge need concrete contracts.
- **Next action:** finalize the backend interface with the first Win32 fake/real implementation.
- **Related:** [Runtime](../notes/runtime.md), [Sandbox](../notes/sandbox.md), [Q-23](../decisions/execution.md#q-23)

## OI-18 Scheduler defaults and measurement freshness

- **Status:** open
- **Source:** [Scheduler](../spec/07-scheduler-v1.md#open-issues)
- **Why:** default reservations and freshness windows are deployment policy, not wire constants.
- **Next action:** choose development defaults and validate the conservative accounting against real workloads.
- **Related:** [Scheduler](../notes/scheduler.md), [Q-16](../decisions/control-plane.md#q-16)

## OI-19 Artifact manifest and package resolution

- **Status:** open
- **Source:** [Store](../spec/08-artifact-store-protocol.md#open-issues)
- **Why:** canonical manifests, package registration/resolution, transfer schemas and leases are still open.
- **Next action:** define canonical encoding and resolution behavior before implementing the store.
- **Related:** [Artifact store](../notes/artifact-store.md), [Package](../notes/package.md)

## OI-20 Cross-host ABI package mapping

- **Status:** open
- **Source:** [Store](../spec/08-artifact-store-protocol.md#open-issues)
- **Why:** one ABI/architecture key must select executable bits without adding host OS as a scheduling dimension.
- **Next action:** prototype the mapping on Windows, Linux and DRIVE OS.
- **Related:** [Package](../notes/package.md), [OI-04](#oi-04-portable-native-abi)

## OI-21 License and platform validation

- **Status:** open
- **Source:** [Repository](../spec/09-repository-cmake-structure.md#license-and-attribution)
- **Why:** publication needs a standard free attribution license; DRIVE OS needs actual toolchain validation.
- **Next action:** evaluate MIT/BSD-3/Apache-2.0 and validate vcpkg + C++23 on the real SDK.
- **Related:** [Q-8](../decisions/scope.md#q-8), [Q-38](../decisions/engineering.md#q-38)

## OI-22 Dependency enforcement and toolchain pinning

- **Status:** open
- **Source:** [Repository](../spec/09-repository-cmake-structure.md#mechanical-enforcement)
- **Why:** component boundaries are only real if the build graph and includes enforce them.
- **Next action:** implement the CMake file-API allowlist check and pin compiler/dependency versions.
- **Related:** [Q-39](../decisions/engineering.md#q-39), [Q-43](../decisions/engineering.md#q-43), [Repository structure](../notes/repository-structure.md)

## OI-23 Client authorization and local isolation

- **Status:** open
- **Source:** [Security](../spec/10-security-model.md#open-issues)
- **Why:** V1 has no TLS or strong peer authorization, and local IPC isolation is host-dependent.
- **Next action:** specify client authorization, per-platform filesystem/IPC safety and audit expectations.
- **Related:** [Security](../notes/security.md), [Q-41](../decisions/engineering.md#q-41)

## OI-24 Hard isolation threat model

- **Status:** open
- **Source:** [Security](../spec/10-security-model.md#open-issues)
- **Why:** the sandbox is operational, not a security boundary; untrusted workloads are out of scope until proven otherwise.
- **Next action:** define and validate a hard-isolation threat model before claiming confinement.
- **Related:** [Sandbox](../notes/sandbox.md), [Security](../notes/security.md)

## OI-25 Network stack selection

- **Status:** open
- **Source:** [Cross-cutting](../architecture/07-cross-cutting.md#71-technology-choices-v1), [Wire protocol](../spec/05-wire-protocol.md#serialization-benchmark-and-selection)
- **Why:** async networking choice affects portability, coroutine integration, performance and dependencies.
- **Next action:** benchmark candidate stacks with the control framing and loopback/LAN latency targets.
- **Related:** [Q-40](../decisions/engineering.md#q-40), [Fabric](../notes/fabric.md)

## OI-26 Workspace snapshot adoption

- **Status:** proposed
- **Source:** [Workspace cache idea](../../todo/content-addressed-workspace-cache.md), [Store](../spec/08-artifact-store-protocol.md)
- **Why:** sending only missing blobs could sharply reduce repeated build/test transfer volume.
- **Next action:** decide whether to adopt workspace trees/snapshots and update the store specs rather than treating the idea note as design.
- **Related:** [Workspace cache note](../notes/workspace-cache.md), [Artifact store](../notes/artifact-store.md)

## OI-27 Test and dependency baseline

- **Status:** open
- **Source:** [Repository](../spec/09-repository-cmake-structure.md#dependencies-and-toolchains), [Cross-cutting](../architecture/07-cross-cutting.md#75-testing-strategy)
- **Why:** the current tests use a dependency-free harness; vcpkg and GoogleTest were suggested but not adopted.
- **Next action:** choose the manifest/baseline and decide whether to wrap or replace the harness.
- **Related:** [Testing](../notes/testing.md), [Repository structure](../notes/repository-structure.md)

## OI-28 Minimal hello resolution

- **Status:** open
- **Source:** [Store](../spec/08-artifact-store-protocol.md#submission-inputs-and-outputs), [Roadmap](../architecture/08-roadmap.md#milestones)
- **Why:** the first vertical slice needs an executable/package resolution path before the master can queue work.
- **Next action:** define the smallest `hello` package or ephemeral artifact path and its ABI/architecture metadata.
- **Related:** [Package](../notes/package.md), [Implementation map](../maps/implementation.md)

## Adding a question

Use [`templates/question.md`](../templates/question.md). Give it the next `OI-*` number, a source,
a concrete next action and links to affected notes/decisions. Then add it to the theme groups in
[`maps/open-questions.md`](../maps/open-questions.md).
