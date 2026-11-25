---
type: guide
id: knowledge-base
status: active
tags: [guide, second-brain, workflow]
aliases: ["Second brain", "Knowledge base guide"]
last_reviewed: 2026-11-25
---

# Knowledge base guide

> The documentation is a linked second brain, not a folder of linear reports. The architecture and
> specification documents remain the single source of truth; the notes, maps and records make that
> knowledge easy to enter, traverse and maintain.

## Why this exists

Pluribus has many coupled decisions: execution semantics, scheduling, networking, artifacts,
security and platform boundaries. Reading the repository top-to-bottom is a poor way to answer a
single question or to see what is still undecided. The second-brain layer adds:

- **entry points** for different intents (understand, implement, decide, debug);
- **atomic notes** that answer one question and link to the canonical detail;
- **decision records** that make accepted choices linkable and revisitable;
- **open-question registers** so uncertainty is visible instead of buried;
- **an implementation log** so progress and next actions survive sessions.

## Layers

| Layer | Location | Purpose | Source of truth? |
|---|---|---|---|
| Home | [`README.md`](README.md) | Dashboard and start-here links | No |
| Maps of content | [`maps/`](maps/README.md) | Curated routes through the knowledge | No |
| Notes | [`notes/`](notes/README.md) | Atomic concept summaries | No; links to canonical docs |
| Canonical design | [`architecture/`](architecture/01-overview.md), [`spec/`](spec/01-execution-model.md) | Architecture and specifications | **Yes** |
| Decisions | [`decisions/`](decisions/README.md) | Themed ADR-style decision files | **Yes** for decision text |
| Questions | [`questions/`](questions/README.md) | Open-issue register | **Yes** for status |
| Reports | [`reports/`](reports/README.md) | Per-task evidence and review | No |
| Log | [`log/`](log/README.md) | Session journal and progress | No |
| Templates | [`templates/`](templates/note.md) | Creation templates | No |
| References | [`references/`](references/README.md) | External standards and sources | No |
| Inbox | [`inbox.md`](inbox.md) | Untriaged capture | No |
| Archive | [`archive/`](archive/README.md) | Retired or superseded material | No |

## The method: capture, organize, distill, express

This repository uses a CODE-style loop, with PARA-like areas and Zettelkasten-like linking:

1. **Capture** — put loose ideas, observations and questions in [`inbox.md`](inbox.md) or add a
   question to [`questions/README.md`](questions/README.md). Do not force them into a spec yet.
2. **Organize** — triage the capture: create or update a note, question, decision or map.
3. **Distill** — write the smallest useful summary, link to the canonical document, and record the
   status and maturity. Prefer one strong link over duplicated prose.
4. **Express** — turn stable knowledge into a specification section, a decision record, code, a
   test, or an implementation-log entry.
5. **Review** — revisit maps and questions at each milestone; promote growing notes to evergreen
   or archive them.

### Capture

- New idea or concern → [`inbox.md`](inbox.md).
- New unresolved design issue → an `OI-*` entry in [`questions/README.md`](questions/README.md).
- New implementation detail worth remembering → a note in [`notes/`](notes/README.md).
- A decision actually made → a record in [`decisions/`](decisions/README.md).

### Distill

A note is not a second spec. It should contain:

- a one-sentence answer;
- only the key points needed to orient a reader;
- links to the canonical document and related notes;
- the open questions that still affect it.

When the note and the spec disagree, the spec wins and the note is corrected.

### Express

- Stable behavior goes into `docs/spec/`.
- Stable structure goes into `docs/architecture/`.
- Stable choices go into a decision record and the decision map.
- Stable implementation status goes into [`maps/implementation.md`](maps/implementation.md) and the
  current implementation log.
- Code changes should update the relevant note or map when they change understanding.

## Note metadata

Every note, map, decision, question index and log uses YAML frontmatter:

```yaml
---
type: note            # home | map | note | decision | index | guide | log | template | reference
id: scheduler         # stable local identifier
status: active        # draft | active | implemented | open | resolved | accepted | superseded | archived
maturity: evergreen   # seed | growing | evergreen (notes and maps)
tags: [note, scheduler]
aliases: ["Scheduler V1"]
sources: ["../spec/07-scheduler-v1.md"]
last_reviewed: 2026-11-25
---
```

All fields are optional for templates, but published pages should set at least `type`, `status`,
`tags` and `last_reviewed`.

### Status vocabulary

| Status | Meaning |
|---|---|
| `draft` | Written down but not reviewed or agreed |
| `active` | Current working knowledge |
| `implemented` | Reflected in code and tests |
| `open` | Question or gap still unresolved |
| `resolved` | Question answered; see the linked decision or spec |
| `accepted` | Decision is in force |
| `superseded` | Replaced by a newer note/decision; keep for history |
| `archived` | Retained only for historical context |

### Maturity

- **seed** — incomplete thought; may be wrong.
- **growing** — useful and reviewed, but still changing.
- **evergreen** — stable summary of a stable part of the system.

## Linking rules

- Use **relative Markdown links**, not wiki links, so the same docs render on GitHub and in
  Obsidian. Example: `[Scheduler](spec/07-scheduler-v1.md)`.
- Link to a heading when possible: `[Scheduler filters](spec/07-scheduler-v1.md#filter-admission-and-score)`.
- Every note must be reachable from at least one map. Maps are curated; tags are for search.
- Keep a `## Related` section on notes and a `## Canonical sources` section so backlinks are
  meaningful without requiring a plugin.
- Never copy a whole specification into a note. Summarize and link.
- When a decision changes, update the decision record, the decision map and the affected notes in
  the same change.

## Naming

| Kind | Convention | Example |
|---|---|---|
| Map | `kebab-case.md` | `implementation.md` |
| Note | `kebab-case.md` | `execution-lifecycle.md` |
| Decision | `theme.md#q-n` | `execution.md#q-10` |
| Question | `OI-NN` entry | `OI-01` |
| Log | `YYYY-MM-DD.md` | `2026-11-25.md` |
| Template | `type.md` | `decision.md` |

## Workflows

### Add a note

1. Create `notes/<topic>.md` from [`templates/note.md`](templates/note.md).
2. Add it to the relevant map and [`maps/knowledge.md`](maps/knowledge.md).
3. Link it from related notes and from the canonical document if useful.
4. Set `status` and `maturity` honestly.

### Add or resolve a question

1. Add an `OI-*` entry to [`questions/README.md`](questions/README.md) with source and next action.
2. Put it in a theme group in [`maps/open-questions.md`](maps/open-questions.md).
3. When resolved, change status to `resolved`, link the decision or spec section, and update the
   maps. Do not delete the entry; history matters.

### Record a decision

1. Add a `Q-N` section to the relevant themed file in [`decisions/`](decisions/README.md), using
   [`templates/decision.md`](templates/decision.md) as a guide.
2. Add the row to [`decisions/README.md`](decisions/README.md) and the theme in
   [`maps/decisions.md`](maps/decisions.md).
3. Link the affected note and spec.
4. If it replaces an earlier decision, mark the old one `superseded` and link both ways.

### Log a session

1. Create `log/YYYY-MM-DD.md` from [`templates/log.md`](templates/log.md).
2. Record what changed, how it was verified, what was learned, and the next action.
3. Link touched notes, decisions and implementation-map changes.

### Prepare a task for review

1. Create the task branch from `dev` and keep work off `dev`/`master`.
2. When the task is implemented and verified, create `reports/<task-slug>.md` from
   [`templates/report.md`](templates/report.md).
3. Include verification commands/results and measurements/benchmarks when relevant.
4. Update [`reports/README.md`](reports/README.md), [`maps/reports.md`](maps/reports.md) and the
   [implementation log](log/README.md).
5. Stop and ask the user to review; only the user merges into `dev`.

## Review cadence

- **Per session:** update the log and the implementation map; triage the inbox.
- **Per milestone:** review open questions, decisions and notes touched by the milestone; retire
  stale assumptions.
- **Before a release or architecture change:** walk the architecture map and the decision map for
  contradictions or missing links.

## Tooling

- **GitHub:** the docs render as normal Markdown; relative links and anchors work.
- **Obsidian:** open `docs/` as a vault. Standard Markdown links are recognized, so the graph and
  backlinks work without committing tool-specific configuration. No plugin is required.
- **Editors:** frontmatter is valid YAML and is ignored by GitHub rendering.
- **Search:** use the repository search or Obsidian search; tags and aliases improve retrieval.

## Templates

- [Note](templates/note.md)
- [Decision record](templates/decision.md)
- [Map of content](templates/map.md)
- [Question](templates/question.md)
- [Log entry](templates/log.md)
- [Task report](templates/report.md)
- [Architecture/spec note](templates/spec.md)

## Quality checklist

- [ ] Frontmatter has `type`, `status`, `tags` and `last_reviewed`.
- [ ] The page is linked from at least one map.
- [ ] Canonical sources are linked with anchors where useful.
- [ ] No duplicated source-of-truth text.
- [ ] Related notes and open questions are linked.
- [ ] Status and maturity match reality.
- [ ] The implementation map is updated if code changed.
- [ ] The decision map is updated if a choice was made.
