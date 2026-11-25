---
type: template
id: template-report
status: active
tags: [template, report, review]
last_reviewed: 2026-11-25
---

# Task report: {{Task title}}

## Summary

{{One paragraph: what was done, why, and the current readiness state.}}

## Scope

- **Branch:** `{{task/slug}}`
- **Base:** `dev`
- **Commits:** {{hash(es) or "see branch history"}}
- **Date:** {{YYYY-MM-DD}}
- **Status:** review

## What changed

- 

## Verification

| Check | Command | Result |
|---|---|---|
| Build | `{{command}}` | {{pass/fail}} |
| Unit tests | `{{command}}` | {{pass/fail, counts}} |
| Integration tests | `{{command}}` | {{pass/fail or not applicable}} |
| Static/link checks | `{{command}}` | {{pass/fail}} |

Include the exact preset, platform, compiler and configuration when relevant.

## Measurements and benchmarks

Use this section when the task affects performance, throughput, latency, memory, binary size or build
time. Otherwise write: **Not applicable — this is a documentation/process change.**

| Metric | Before | After | Notes |
|---|---:|---:|---|
| {{metric}} | {{value}} | {{value}} | {{environment/method}} |

Record the benchmark command, dataset, hardware, compiler, build type and number of runs. Do not
claim results that were not measured.

## Risks and limitations

- 

## Follow-up work

- 

## Review checklist

- [ ] The branch builds and the relevant tests pass.
- [ ] The report matches the actual diff.
- [ ] Measurements are reproducible or explicitly marked not applicable.
- [ ] No secrets, host-specific paths or generated build output are committed.
- [ ] The task does not modify `dev` or `master` directly.
- [ ] The user has enough evidence to approve or request changes.

## Links

- [Task reports index](../reports/README.md)
- [Implementation map](../maps/implementation.md)
- Related notes, decisions or questions:
