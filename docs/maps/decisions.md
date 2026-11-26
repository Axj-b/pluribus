---
type: map
id: decisions-map
status: active
tags: [map, decision, adr]
aliases: ["Decision MOC", "ADR map"]
last_reviewed: 2026-11-25
---

# Decision map

> A thematic view of the six themed decision files. The full ordered index is in
> [Decision records](../decisions/README.md); each linked section points to the canonical document.

## Scope and product

- [Q-1](../decisions/scope.md#q-1) — RadarScape is the first consumer; the platform stays domain-agnostic.
- [Q-2](../decisions/scope.md#q-2) — target 2–10 nodes and well below 1000 concurrent executions initially.
- [Q-7](../decisions/scope.md#q-7) — start from scratch; no RadarScape coupling in the platform.
- [Q-8](../decisions/scope.md#q-8) — name Pluribus, namespace `pluribus::`, CLI `plu`; choose a free attribution license.
- [Q-9](../decisions/scope.md#q-9) — timeline/team undecided; development is milestone-driven.
- [Q-45](../decisions/scope.md#q-45) — Pluribus is a uniform distributed environment, not a Linux simulator.
- [Q-46](../decisions/scope.md#q-46) — write ordered specs, then implement the first remote `hello` before expanding.

## Execution and core model

- [Q-10](../decisions/execution.md#q-10) — persisted node UUID plus separate name; immutable execution identity plus friendly ID.
- [Q-11](../decisions/execution.md#q-11) — optional opaque labels for correlation; the scheduler never interprets them.
- [Q-12](../decisions/execution.md#q-12) — separate timestamped stdout/stderr with bounded history, replay then live.
- [Q-13](../decisions/execution.md#q-13) — reconcile active executions on reconnect; apply a configured orphan policy.
- [Q-21](../decisions/execution.md#q-21) — execution and client lifetimes are separate; detach is supported.
- [Q-23](../decisions/execution.md#q-23) — prefer working-directory-relative paths and environment bridges; no V1 filesystem virtualization.
- [Q-26](../decisions/execution.md#q-26) — configurable sandbox retention; persistent artifacts are stored separately.
- [Q-27](../decisions/execution.md#q-27) — model GPU capabilities/resources for future allocation, without complex allocation in V1.

## Control plane, scheduling and health

- [Q-16](../decisions/control-plane.md#q-16) — resources are optional; unknown is not zero; master defaults apply for accounting.
- [Q-17](../decisions/control-plane.md#q-17) — strict FIFO queue; later entries do not bypass a blocked head.
- [Q-18](../decisions/control-plane.md#q-18) — configurable heartbeat, default 1 s interval and 5 missed beats.
- [Q-19](../decisions/control-plane.md#q-19) — nodes keep supervising executions through a master outage.
- [Q-20](../decisions/control-plane.md#q-20) — in-memory master state initially; add durability behind a storage abstraction.
- [Q-25](../decisions/control-plane.md#q-25) — initial Thor target is DRIVE OS Linux; validate its toolchain while keeping core generic.
- [Q-29](../decisions/control-plane.md#q-29) — static master configuration in V1; discovery is optional later.

## Fabric, protocol and services

- [Q-15](../decisions/fabric-services.md#q-15) — benchmark FlatBuffers, Cap'n Proto and Protobuf before choosing a wire codec.
- [Q-28](../decisions/fabric-services.md#q-28) — idempotent submit with client request IDs, sequence numbers and a 1 MiB control limit.
- [Q-30](../decisions/fabric-services.md#q-30) — hierarchical service names, distinct from provider instance identity.
- [Q-31](../decisions/fabric-services.md#q-31) — protocol permits multiple providers; V1 resolves one.
- [Q-32](../decisions/fabric-services.md#q-32) — reliable ordered byte stream is the V1 service minimum.
- [Q-33](../decisions/fabric-services.md#q-33) — explicit READY before resolution; resolve supports waiting/timeout.
- [Q-34](../decisions/fabric-services.md#q-34) — ordinary networking/hardware remain accessible; no general virtual network stack.
- [Q-35](../decisions/fabric-services.md#q-35) — C++23 API plus a documented language-neutral protocol; no C++ layouts on the wire.
- [Q-47](../decisions/fabric-services.md#q-47) — use Protocol Buffers for V1 control payloads after the codec benchmark.

## Store, packaging and artifacts

- [Q-3](../decisions/store-packaging.md#q-3) — compatibility is Pluribus ABI + CPU architecture, not host OS.
- [Q-4](../decisions/store-packaging.md#q-4) — native binaries only in V1; revisit the runtime mechanism after prototyping.
- [Q-5](../decisions/store-packaging.md#q-5) — explicit package/local/artifact syntax; no node PATH lookup or implicit precedence.
- [Q-36](../decisions/store-packaging.md#q-36) — configurable LRU cache; pin active artifacts.
- [Q-37](../decisions/store-packaging.md#q-37) — peer-to-peer artifact transfer long-term; V1 may relay via the master.

## Engineering, API and operations

- [Q-6](../decisions/engineering.md#q-6) — C++23 client API with no stable C ABI initially; avoid implementation-specific types.
- [Q-14](../decisions/engineering.md#q-14) — auto-detect reliable capabilities and supplement with configuration; host OS is not a capability.
- [Q-22](../decisions/engineering.md#q-22) — TOML configuration; foreground operation in V1.
- [Q-24](../decisions/engineering.md#q-24) — C++ service API plus a local language-neutral endpoint; port forwarding later.
- [Q-38](../decisions/engineering.md#q-38) — pin a C++23 toolchain matrix in CI and validate Thor.
- [Q-39](../decisions/engineering.md#q-39) — small maintained dependencies and a reproducible CMake package strategy.
- [Q-40](../decisions/engineering.md#q-40) — async event-driven I/O; coroutines acceptable; small worker pool.
- [Q-41](../decisions/engineering.md#q-41) — trusted-network V1 with join token, explicit bind, validation and size limits.
- [Q-42](../decisions/engineering.md#q-42) — structured logs plus a human-readable interactive console.
- [Q-43](../decisions/engineering.md#q-43) — enforce component boundaries with CMake and CI checks.
- [Q-44](../decisions/engineering.md#q-44) — CLI supports `--json`, explicit config precedence, foreground wait/stream and `--detach`.

## See also

- [Full decision index](../decisions/README.md)
- [Architecture map](architecture.md)
- [Open questions](open-questions.md)
