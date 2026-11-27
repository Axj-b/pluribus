---
type: index
id: reports
status: active
tags: [report, review, process]
aliases: ["Task reports", "Review reports"]
last_reviewed: 2026-11-25
---

# Task reports

> One report per task, written when the work is ready for the user to review. A report is evidence
> and a review aid, not a canonical specification.

## Rules

- Create the report on the task branch before asking for review.
- Use [`templates/report.md`](../templates/report.md).
- Include verification commands and exact results.
- Include measurements or benchmarks when the task affects performance, throughput, latency, memory
  or build time. State explicitly when they are not applicable.
- Record risks, limitations and follow-up work.
- The user decides when the task is done and when to merge the branch into `dev`.

See [AGENTS.md](../../AGENTS.md) for the branch and review workflow.

## Reports

| Task | Branch | Status | Date |
|---|---|---|---|
| [Protobuf handshake schemas](008-protobuf-schemas.md) | `task/protobuf-schemas` | review | 2026-11-27 |
| [Dependency enforcement](007-dependency-enforcement.md) | `task/dependency-enforcement` | review | 2026-11-27 |
| [Async transport benchmark](006-async-transport-benchmark.md) | `task/async-transport-bench` | review | 2026-11-27 |
| [Codec benchmark](005-codec-benchmark.md) | `task/codec-benchmark` | review | 2026-11-26 |
| [Fabric session](004-fabric-session.md) | `task/fabric-session` | review | 2026-11-25 |
| [Fabric transport](003-fabric-transport.md) | `task/fabric-transport` | review | 2026-11-25 |
| [Open question guidance](002-open-question-guidance.md) | `chore/open-question-guidance` | merged | 2026-11-25 |
| [Task workflow scaffold](001-task-workflow.md) | `chore/task-workflow` | merged | 2026-11-25 |

## Status vocabulary

| Status | Meaning |
|---|---|
| `review` | Work is complete and waiting for the user |
| `changes-requested` | User requested more work |
| `approved` | User accepted the task |
| `merged` | User merged the branch into `dev` |

## Map

- [Reports map](../maps/reports.md)
