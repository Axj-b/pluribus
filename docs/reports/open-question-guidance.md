---
type: report
id: report-open-question-guidance
status: review
task: Open question explanations and suggested behaviors
branch: chore/open-question-guidance
base: dev
date: 2026-11-25
tags: [report, question, guidance]
last_reviewed: 2026-11-25
---

# Task report: Open question explanations and suggested behaviors

## Summary

Expanded all 28 entries in the open-question register so each one now explains the problem, proposes
a concrete V1 behavior, records the trade-offs and states a confidence level and next action. The
suggestions are proposals for review, not accepted decisions.

## Scope

- **Branch:** `chore/open-question-guidance`
- **Base:** `dev`
- **Commits:** see `git log --oneline dev..HEAD`
- **Date:** 2026-11-25
- **Status:** review

## What changed

- Expanded every `OI-*` entry in [`docs/questions/README.md`](../questions/README.md) with:
  - a plain-language question;
  - an explanation of why it is unresolved and what it affects;
  - a suggested behavior;
  - trade-offs and residual risk;
  - a confidence level and next action.
- Added a “How to read a suggestion” section that separates proposals from accepted decisions.
- Updated the [open-question map](../maps/open-questions.md), [documentation home](../README.md) and
  [knowledge-base workflow](../knowledge-base.md) to describe the new shape.
- Updated [`templates/question.md`](../templates/question.md) so new questions capture the same
  fields.
- Added this report to the reports index and map, and updated the implementation log.

## Verification

| Check | Command | Result |
|---|---|---|
| Open-question coverage | `Select-String` count of `## OI-` headings | 28 / 28 |
| Suggested behavior coverage | `Select-String` count of `Suggested behavior` | 28 / 28 |
| Documentation links and anchors | `scripts\check-docs.cmd` | see result below |
| Unit tests | `ctest --preset windows-gcc-debug` | 3 / 3 targets passed |
| Working tree | `git status --short` | clean after commit |

Documentation check result:

```text
Checked 75 Markdown files: 1127 relative links and 369 local anchors resolve.
```

## Measurements and benchmarks

**Not applicable — this is a documentation and process change, not a performance change.**

Coverage is the relevant measurement: 28 of 28 open questions have an explanation, a suggested
behavior, trade-offs, confidence and a next action. The documentation checker also reports the
number of Markdown files, relative links and local anchors validated.

## Risks and limitations

- The suggestions are deliberately opinionated. They need review and may be rejected or rewritten.
- Some suggestions interact with accepted decisions:
  - OI-04/OI-20 propose putting runtime/OS detail into the ABI key rather than adding a scheduler
    host-OS field; the exact grammar needs a decision.
  - OI-14 suggests Protocol Buffers as a provisional default, but the Q-15 benchmark is still
    required before the wire format is frozen.
  - OI-21 suggests Apache-2.0 as a default license; the final license choice remains with the user.
- The open-question register is not a decision log. Accepting a suggestion still requires a `Q-N`
  record and a spec update.
- The suggested service-name grammar is stricter than the current provisional parser, so code and
  docs will diverge until the parser is aligned.

## Follow-up work

1. Review the suggestions and mark each `accepted`, `rejected` or `needs discussion`.
2. Turn accepted suggestions into `Q-47`+ decision records and update the affected specs.
3. Align the `ServiceName` parser with OI-12 if that grammar is accepted.
4. Choose the serializer and ABI-key grammar before freezing control payloads or package metadata.
5. Keep the report status at `review` until the user decides; only the user merges into `dev`.

## Review checklist

- [ ] Every question has a useful explanation, not just a restated title.
- [ ] Every suggested behavior is concrete enough to implement or reject.
- [ ] Trade-offs and confidence are honest.
- [ ] Suggestions are clearly marked as proposals, not decisions.
- [ ] The report matches the actual diff.
- [ ] `scripts\check-docs.cmd` and the unit tests pass.
- [ ] No build output, secrets or host-specific paths are committed.
- [ ] `dev` and `master` were not modified directly.

## Links

- [Open questions register](../questions/README.md)
- [Open questions map](../maps/open-questions.md)
- [Task reports index](../reports/README.md)
- [Implementation map](../maps/implementation.md)
- [Agent workflow](../../AGENTS.md)
