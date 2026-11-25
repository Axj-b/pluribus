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
| [Task workflow scaffold](task-workflow.md) | `chore/task-workflow` | merged | 2026-11-25 |

## Status vocabulary

| Status | Meaning |
|---|---|
| `review` | Work is complete and waiting for the user |
| `changes-requested` | User requested more work |
| `approved` | User accepted the task |
| `merged` | User merged the branch into `dev` |

## Map

- [Reports map](../maps/reports.md)
