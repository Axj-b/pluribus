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

- [Codec benchmark](../reports/codec-benchmark.md) — `task/codec-benchmark` — Protobuf/FlatBuffers/Cap'n Proto comparison and Q-47 selection.
- [Fabric session](../reports/fabric-session.md) — `task/fabric-session` — bootstrap handshake and sequenced sessions.
- [Fabric transport](../reports/fabric-transport.md) — `task/fabric-transport` — blocking TCP, loopback and frame-stream transport with benchmarks.

## Approved

- None.

## Merged

- [Open question guidance](../reports/open-question-guidance.md) — `chore/open-question-guidance` — explanations, suggested behaviors and trade-offs for all 28 open questions.
- [Task workflow scaffold](../reports/task-workflow.md) — `chore/task-workflow` — branch/report workflow and templates.

## How to use reports

- Open the report before reviewing the branch.
- Check the verification table and measurements.
- Approve or request changes; the user decides when to merge into `dev`.
- After merging, update the report status to `merged` and move it to the merged group.
