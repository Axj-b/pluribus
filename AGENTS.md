# Agent workflow

These rules apply to every coding task in this repository.

## Branching

- `dev` is the integration branch. Do not work directly on `dev` or `master`.
- Start each task on its own branch from `dev`:
  - `task/<short-slug>` for features and implementation work;
  - `fix/<short-slug>` for bug fixes;
  - `chore/<short-slug>` for tooling, documentation or process work.
- State the branch name and intended scope at the start of the task.
- Commit task work on the task branch. Do not merge or push unless the user asks.

## Task completion report

When implementation and verification are complete, create a report before declaring the task done:

- File: `docs/reports/<task-slug>.md`
- Template: `docs/templates/report.md`
- Update the report index in `docs/reports/README.md`.
- Add the report to the relevant map and to the implementation log.

The report must contain:

- task summary and scope;
- branch and commit information;
- what changed and why;
- verification commands and exact results;
- measurements and benchmarks when the task has performance, throughput, latency, memory or build-time impact;
- risks, limitations and follow-up work;
- a review checklist for the user.

For documentation-only or process-only tasks, state that benchmarks are not applicable and use the
checks that do apply, such as link checks, anchor checks, linting or build verification. Never claim a
benchmark that was not run.

## Done means reviewed, not merged

- The task is **ready for review** when the report exists and the branch is clean and verified.
- The user decides whether the task is done and when to merge the branch into `dev`.
- Do not merge a task branch, update `dev`, or push on the user's behalf.

## Report status

Use the report frontmatter:

- `status: review` while waiting for the user;
- `status: approved` only after the user confirms;
- `status: merged` only after the user merges.

## Verification rules

- Run the narrowest relevant tests, then the full suite when practical.
- For documentation or process changes, run `pwsh -File tools/check-docs.ps1`.
- Record the exact preset/command and the pass/fail result.
- Measure before and after when changing performance-sensitive code; include environment details.
- Check `git status` before calling the task ready for review.
- Do not hide skipped, flaky or unavailable checks.

## Documentation rules

- Keep the canonical architecture and specification docs authoritative.
- Update the second-brain links, maps, decisions or questions when the task changes understanding.
- Use `docs/reports/` for task evidence, not for canonical design.
