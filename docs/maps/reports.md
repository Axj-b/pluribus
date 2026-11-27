---
type: map
id: reports-map
status: active
tags: [map, report, review]
aliases: ["Task reports MOC"]
last_reviewed: 2026-11-25
---

# Reports map

> Task reports waiting for review, approved, or merged. See the
> [reports index](../reports/README.md) for the canonical list and status.

## In review

- None.

## Approved

- None.

## Merged

- [Protobuf handshake schemas](../reports/008-protobuf-schemas.md) — `task/protobuf-schemas` — versioned Protobuf Hello/Welcome/Reject codec.
- [Dependency enforcement](../reports/007-dependency-enforcement.md) — `task/dependency-enforcement` — CMake graph check and platform-include scan.
- [Async transport benchmark](../reports/006-async-transport-benchmark.md) — `task/async-transport-bench` — blocking vs Asio single-connection comparison.
- [Codec benchmark](../reports/005-codec-benchmark.md) — `task/codec-benchmark` — Protobuf/FlatBuffers/Cap'n Proto comparison and Q-47 selection.
- [Fabric session](../reports/004-fabric-session.md) — `task/fabric-session` — bootstrap handshake and sequenced sessions.
- [Fabric transport](../reports/003-fabric-transport.md) — `task/fabric-transport` — blocking TCP, loopback and frame-stream transport with benchmarks.
- [Open question guidance](../reports/002-open-question-guidance.md) — `chore/open-question-guidance` — explanations, suggested behaviors and trade-offs for all 28 open questions.
- [Task workflow scaffold](../reports/001-task-workflow.md) — `chore/task-workflow` — branch/report workflow and templates.

## How to use reports

- Open the report before reviewing the branch.
- Check the verification table and measurements.
- Approve or request changes; the user decides when to merge into `dev`.
- After merging, update the report status to `merged` and move it to the merged group.
