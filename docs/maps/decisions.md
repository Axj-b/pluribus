---
type: map
id: decisions-map
status: active
tags: [map, decision, adr]
aliases: ["Decision MOC", "ADR map"]
last_reviewed: 2026-11-25
---

# Decision map

> A thematic view of accepted decisions. The full ordered index is in
> [Decision records](../decisions/README.md); each linked record points to the canonical document.

## Scope and product

- [Q-1](../decisions/Q-01.md) — RadarScape is the first consumer; the platform stays domain-agnostic.
- [Q-2](../decisions/Q-02.md) — target 2–10 nodes and well below 1000 concurrent executions initially.
- [Q-7](../decisions/Q-07.md) — start from scratch; no RadarScape coupling in the platform.
- [Q-8](../decisions/Q-08.md) — name Pluribus, namespace `pluribus::`, CLI `plu`; choose a free attribution license.
- [Q-9](../decisions/Q-09.md) — timeline/team undecided; development is milestone-driven.
- [Q-45](../decisions/Q-45.md) — Pluribus is a uniform distributed environment, not a Linux simulator.
- [Q-46](../decisions/Q-46.md) — write ordered specs, then implement the first remote `hello` before expanding.

## Execution and core model

- [Q-10](../decisions/Q-10.md) — persisted node UUID plus separate name; immutable execution identity plus friendly ID.
- [Q-11](../decisions/Q-11.md) — optional opaque labels for correlation; the scheduler never interprets them.
- [Q-12](../decisions/Q-12.md) — separate timestamped stdout/stderr with bounded history, replay then live.
- [Q-13](../decisions/Q-13.md) — reconcile active executions on reconnect; apply a configured orphan policy.
- [Q-21](../decisions/Q-21.md) — execution and client lifetimes are separate; detach is supported.
- [Q-23](../decisions/Q-23.md) — prefer working-directory-relative paths and environment bridges; no V1 filesystem virtualization.
- [Q-26](../decisions/Q-26.md) — configurable sandbox retention; persistent artifacts are stored separately.
- [Q-27](../decisions/Q-27.md) — model GPU capabilities/resources for future allocation, without complex allocation in V1.

## Control plane, scheduling and health

- [Q-16](../decisions/Q-16.md) — resources are optional; unknown is not zero; master defaults apply for accounting.
- [Q-17](../decisions/Q-17.md) — strict FIFO queue; later entries do not bypass a blocked head.
- [Q-18](../decisions/Q-18.md) — configurable heartbeat, default 1 s interval and 5 missed beats.
- [Q-19](../decisions/Q-19.md) — nodes keep supervising executions through a master outage.
- [Q-20](../decisions/Q-20.md) — in-memory master state initially; add durability behind a storage abstraction.
- [Q-25](../decisions/Q-25.md) — initial Thor target is DRIVE OS Linux; validate its toolchain while keeping core generic.
- [Q-29](../decisions/Q-29.md) — static master configuration in V1; discovery is optional later.

## Fabric, protocol and services

- [Q-15](../decisions/Q-15.md) — benchmark FlatBuffers, Cap'n Proto and Protobuf before choosing a wire codec.
- [Q-28](../decisions/Q-28.md) — idempotent submit with client request IDs, sequence numbers and a 1 MiB control limit.
- [Q-30](../decisions/Q-30.md) — hierarchical service names, distinct from provider instance identity.
- [Q-31](../decisions/Q-31.md) — protocol permits multiple providers; V1 resolves one.
- [Q-32](../decisions/Q-32.md) — reliable ordered byte stream is the V1 service minimum.
- [Q-33](../decisions/Q-33.md) — explicit READY before resolution; resolve supports waiting/timeout.
- [Q-34](../decisions/Q-34.md) — ordinary networking/hardware remain accessible; no general virtual network stack.
- [Q-35](../decisions/Q-35.md) — C++23 API plus a documented language-neutral protocol; no C++ layouts on the wire.

## Store, packaging and artifacts

- [Q-3](../decisions/Q-03.md) — compatibility is Pluribus ABI + CPU architecture, not host OS.
- [Q-4](../decisions/Q-04.md) — native binaries only in V1; revisit the runtime mechanism after prototyping.
- [Q-5](../decisions/Q-05.md) — explicit package/local/artifact syntax; no node PATH lookup or implicit precedence.
- [Q-36](../decisions/Q-36.md) — configurable LRU cache; pin active artifacts.
- [Q-37](../decisions/Q-37.md) — peer-to-peer artifact transfer long-term; V1 may relay via the master.

## Runtime, engineering and operations

- [Q-6](../decisions/Q-06.md) — C++23 client API with no stable C ABI initially; avoid implementation-specific types.
- [Q-14](../decisions/Q-14.md) — auto-detect reliable capabilities and supplement with configuration; host OS is not a capability.
- [Q-22](../decisions/Q-22.md) — TOML configuration; foreground operation in V1.
- [Q-24](../decisions/Q-24.md) — C++ service API plus a local language-neutral endpoint; port forwarding later.
- [Q-38](../decisions/Q-38.md) — pin a C++23 toolchain matrix in CI and validate Thor.
- [Q-39](../decisions/Q-39.md) — small maintained dependencies and a reproducible CMake package strategy.
- [Q-40](../decisions/Q-40.md) — async event-driven I/O; coroutines acceptable; small worker pool.
- [Q-41](../decisions/Q-41.md) — trusted-network V1 with join token, explicit bind, validation and size limits.
- [Q-42](../decisions/Q-42.md) — structured logs plus a human-readable interactive console.
- [Q-43](../decisions/Q-43.md) — enforce component boundaries with CMake and CI checks.
- [Q-44](../decisions/Q-44.md) — CLI supports `--json`, explicit config precedence, foreground wait/stream and `--detach`.

## See also

- [Full decision index](../decisions/README.md)
- [Architecture map](architecture.md)
- [Open questions](open-questions.md)
