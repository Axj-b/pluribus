---
type: report
id: report-task-workflow
status: review
task: Task workflow scaffold
branch: chore/task-workflow
base: dev
date: 2026-11-25
tags: [report, process, workflow]
last_reviewed: 2026-11-25
---

# Task report: Task workflow scaffold

## Summary

Set up the branch-and-report workflow requested for future tasks. `dev` is now the integration
branch, task work happens on dedicated branches, and every task finishes with a report in
`docs/reports/` before the user decides whether to merge. This branch also adds the report template,
the reports index/map and a reproducible documentation link checker.

## Scope

- **Branch:** `chore/task-workflow`
- **Base:** `dev` (created from the current `master` at `72bce85`)
- **Commits:** see `git log --oneline dev..HEAD`
- **Date:** 2026-11-25
- **Status:** review

## What changed

- Added [`AGENTS.md`](../../AGENTS.md) with the branch, verification, reporting and review rules.
- Added [`docs/reports/README.md`](../reports/README.md) as the per-task report index.
- Added [`docs/templates/report.md`](../templates/report.md) for task evidence and review.
- Added [`docs/maps/reports.md`](../maps/reports.md) as the reports map.
- Added [`tools/check-docs.ps1`](../../tools/check-docs.ps1) to verify relative links and local
  anchors across the docs.
- Updated the documentation home, knowledge-base guide, maps index and implementation log to point
  at the new report workflow.
- Kept the canonical architecture/spec documents authoritative; task evidence lives separately in
  `docs/reports/`.

## Verification

| Check | Command | Result |
|---|---|---|
| Branch | `git branch --show-current` | `chore/task-workflow` |
| Base branch | `git merge-base --is-ancestor dev HEAD` | pass |
| Documentation links and anchors | `pwsh -File tools/check-docs.ps1` | see result below |
| Unit tests | `ctest --preset windows-gcc-debug` | 3/3 targets passed |
| Working tree | `git status --short` | clean after commit |

Documentation check result:

```text
Checked 74 Markdown files: 1110 relative links and 369 local anchors resolve.
```

## Measurements and benchmarks

**Not applicable — this is a process/documentation change, not a performance change.**

The only quantitative check is the documentation validation pass: number of Markdown files,
relative links and local anchors checked by `tools/check-docs.ps1`.

## Risks and limitations

- `dev` currently points at the same commit as `master` and `origin/dev`; branch protection, push
  timing and remote policy are left to the user.
- The report workflow adds a small amount of overhead per task.
- Reports can go stale if a task continues after the report is written; update the report on every
  material change before asking for review.
- `tools/check-docs.ps1` uses an approximate GitHub heading slugger; unusual heading punctuation may
  need adjustment.

## Follow-up work

- Add the docs check to CI once the dependency/build pipeline exists.
- Decide whether `dev` should be pushed to `origin` and protected.
- After the user approves this task, update the report status to `approved`; after merging, mark it
  `merged` and move it in the reports map.
- Apply the same branch/report workflow to the next implementation task.

## Review checklist

- [ ] The branch is created from `dev`, not from `master`.
- [ ] `AGENTS.md` captures the branch/report/review rules correctly.
- [ ] The reports index, map and template are usable.
- [ ] `pwsh -File tools/check-docs.ps1` passes.
- [ ] The report matches the actual diff.
- [ ] No build output, secrets or host-specific paths are committed.
- [ ] `dev` and `master` were not modified directly.
- [ ] The user approves; only the user merges into `dev`.

## Links

- [Task reports index](../reports/README.md)
- [Reports map](../maps/reports.md)
- [Implementation map](../maps/implementation.md)
- [Knowledge base guide](../knowledge-base.md)
- [Agent workflow](../../AGENTS.md)
