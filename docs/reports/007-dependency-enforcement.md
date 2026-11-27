---
type: report
id: report-dependency-enforcement
status: merged
task: Dependency enforcement
branch: task/dependency-enforcement
base: task/async-transport-bench
merged_into: dev
merged_at: 2026-11-27
date: 2026-11-27
tags: [report, cmake, dependencies, build]
last_reviewed: 2026-11-27
---

# Task report: Dependency enforcement

## Summary

Added two automated checks that make the documented Pluribus dependency direction enforceable:
a configure-time CMake target-graph check (forbidden edges, cycles and unclassified targets) and a
parameterless include scan that rejects platform headers in core, scheduler and fabric public
headers. The branch is stacked on the unmerged async/transport/codec/session branches.

## Scope

- **Branch:** `task/dependency-enforcement`
- **Base:** `task/async-transport-bench` (stacked; merge order matters)
- **Commits:** see `git log --oneline task/async-transport-bench..HEAD`
- **Date:** 2026-11-27
- **Status:** merged

## What changed

- Added `cmake/PluribusDependencies.cmake`:
  - allowlist from the spec target table;
  - normalizes aliases and `$<BUILD_INTERFACE:...>` / `$<INSTALL_INTERFACE:...>` /
    `$<LINK_ONLY:...>` generator expressions;
  - checks direct and transitive Pluribus edges, cycles and unclassified internal targets;
  - fails closed on generator expressions it cannot evaluate.
- Added parameterless `scripts/check-deps.ps1` / `scripts/check-deps.cmd`:
  - scans `core/`, `master/scheduler/` and `fabric/include/` for platform includes.
- Registered a `deps.static` CTest on Windows.
- Updated the scripts README, implementation map and this log.

## Verification

| Check | Command | Result |
|---|---|---|
| Windows configure | `cmake --preset windows-gcc-debug` | graph check passed |
| WSL configure | `cmake --preset wsl-gcc-debug` | graph check passed |
| Windows debug tests | `ctest --preset windows-gcc-debug` | 4/4 passed (includes `deps.static`) |
| Windows release tests | `ctest --preset windows-gcc-release` | 4/4 passed |
| WSL debug tests | `ctest --preset wsl-gcc-debug` | 3/3 passed |
| Include scan | `scripts\check-deps.cmd` | passed, no forbidden includes |
| Documentation | `scripts\check-docs.cmd` | see below |
| Working tree | `git status --short` | clean after commit |

Documentation check result:

```text
Checked 82 Markdown files: 1203 relative links and 386 local anchors resolve.
```

## Measurements and benchmarks

**Not applicable — this is a build/tooling change.**

Coverage figures: the graph allowlist covers 16 Pluribus targets; the include scan covers the
`core/` tree, `master/scheduler/` and `fabric/include/` with 16 forbidden header patterns. No
violations were found on the current graph.

## Risks and limitations

- The allowlist is maintained by hand; a new component must be added explicitly.
- Generator-expression handling is limited to the three documented wrappers; unknown expressions
  fail the configure rather than being silently skipped.
- The include scan is textual and scans only the listed roots; it does not resolve transitive
  compiler dependency files or check the transport implementation sources (which are allowed to use
  platform headers).
- Test and benchmark targets are given a broader allowlist so they can reach `core` through their
  component libraries; that is intentional but should be reviewed when those targets grow.
- No negative fixtures exist yet, so a regression in the checker itself is not covered.
- Toolchain/dependency pinning (OI-22 remainder) is still open.

## Follow-up work

1. Add negative fixtures that deliberately introduce a forbidden edge, cycle and unclassified target.
2. Extend the include scan to transitive compiler dependency files in CI.
3. Pin compiler/preset and dependency versions.
4. Decide the vcpkg/test baseline (OI-27).
5. Wire the checks into CI so they run on every platform, not only configure-time.
6. After the Protobuf schemas are added, keep the `protobuf` dependency inside the allowlist.

## Review checklist

- [ ] The allowlist matches `docs/spec/09` at the time of writing.
- [ ] The graph check fails closed instead of skipping unknown edges.
- [ ] The include scan roots and patterns are correct.
- [ ] The report matches the actual diff.
- [ ] Windows/WSL configure and tests pass, including `deps.static`.
- [ ] No generated build output, secrets or host-specific paths are committed.
- [ ] `dev` and `master` were not modified directly.
- [ ] The user approves; only the user merges into `dev`.

## Links

- [Dependency check source](../../cmake/PluribusDependencies.cmake)
- [Include scan source](../../scripts/check-deps.ps1)
- [Repository structure spec](../spec/09-repository-cmake-structure.md)
- [OI-22 dependency enforcement](../questions/README.md#oi-22-dependency-enforcement-and-toolchain-pinning)
- [Repository structure note](../notes/repository-structure.md)
- [Agent workflow](../../AGENTS.md)
