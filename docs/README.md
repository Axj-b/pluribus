---
type: home
id: home
status: active
tags: [home, dashboard, map, second-brain]
aliases: ["Docs home", "Documentation home", "Second brain"]
last_reviewed: 2026-11-25
---

# Pluribus documentation

> A linked second brain for the Pluribus architecture, specifications, decisions and implementation.
> The canonical design lives in [`architecture/`](architecture/01-overview.md) and [`spec/`](spec/01-execution-model.md);
> the maps, notes and records make it navigable.

Source idea: [`../IDEA.md`](../IDEA.md), retained as the original idea. These documents record the
current decisions. The answered questionnaire is a local, not committed source note.

Status: **draft architecture; core model, scheduler and fabric framing implemented and unit-tested;
ABI/runtime validation pending.**

## Start here

| I want to... | Go to |
|---|---|
| Understand the system in 10 minutes | [Architecture map](maps/architecture.md) → [Overview](architecture/01-overview.md) |
| Find the exact contract for a behavior | [Specification map](maps/specifications.md) |
| See what exists in code and what is next | [Implementation map](maps/implementation.md) |
| Know why a choice was made | [Decision map](maps/decisions.md) |
| See what is still unknown | [Open questions](maps/open-questions.md) |
| Browse concepts by topic | [Knowledge map](maps/knowledge.md), [Notes](notes/README.md) |
| Look up a term | [Glossary](glossary.md) |
| Understand this docs system | [Knowledge base guide](knowledge-base.md) |
| Capture a loose idea | [Inbox](inbox.md) |
| Read the latest session | [Implementation log](log/README.md) |

## Current state

- **Implemented:** `pluribuscore` model, `pluribusscheduler` V1, `pluribusfabric` control framing.
- **Next:** TCP transport/session handshake, dependency enforcement, serializer decision, then a
  master registry/queue and node agent for the first `plu run hello`.
- **Not started:** master, node, runtime backends, store, client, CLI.
- **Tests:** 48 unit tests across core, scheduler and fabric; see
  [Implementation map](maps/implementation.md).

## How this knowledge base works

The docs use a second-brain loop: **capture → organize → distill → express**. The specs remain the
single source of truth; notes summarize and link, decisions record why, questions record uncertainty,
and the implementation log records progress. Read the [knowledge base guide](knowledge-base.md) for
the metadata schema, linking rules, workflows and review cadence.

The same docs work on GitHub and in Obsidian: links are standard relative Markdown, and opening
`docs/` as an Obsidian vault enables graph and backlinks without committing tool configuration.

## Maps of content

| Map | Use it when you want to... |
|---|---|
| [Architecture](maps/architecture.md) | Understand layers, invariants and components |
| [Specifications](maps/specifications.md) | Find the precise contract |
| [Implementation](maps/implementation.md) | See code status, milestones and next actions |
| [Decisions](maps/decisions.md) | Find accepted choices by theme |
| [Open questions](maps/open-questions.md) | Find unresolved design/implementation issues |
| [Knowledge](maps/knowledge.md) | Browse atomic notes by domain and tag |

## Architecture

| # | Document | Content |
|---|----------|---------|
| 1 | [architecture/01-overview.md](architecture/01-overview.md) | Vision, principles, glossary, layering, invariants |
| 2 | [architecture/02-core-model.md](architecture/02-core-model.md) | Identifiers, ExecutionSpec, handle, state machine, node descriptor |
| 3 | [architecture/03-control-plane.md](architecture/03-control-plane.md) | Master, scheduler, registries, failure model |
| 4 | [architecture/04-node-and-runtime.md](architecture/04-node-and-runtime.md) | Node agent, pluribussim, host backend, sandbox |
| 5 | [architecture/05-fabric-and-services.md](architecture/05-fabric-and-services.md) | Transport, protocol, service model and routing |
| 6 | [architecture/06-store-and-packaging.md](architecture/06-store-and-packaging.md) | Artifacts, caching, packages |
| 7 | [architecture/07-cross-cutting.md](architecture/07-cross-cutting.md) | Security, observability, performance, testing, tech stack |
| 8 | [architecture/08-roadmap.md](architecture/08-roadmap.md) | Repository layout, milestones, prototypes, non-goals |

## Specifications in priority order

Decision (Q-46): Develop these planned specifications in order, then implement the first end-to-end
`plu run hello` before expanding into domain workloads or advanced platform features.

1. [Execution model and state machine](spec/01-execution-model.md)
2. [Node/master protocol](spec/02-node-master-protocol.md)
3. [C++23 public API](spec/03-cpp-public-api.md)
4. [Service model and lifecycle](spec/04-service-model.md)
5. [Wire protocol and reconnect/idempotency](spec/05-wire-protocol.md)
6. [Node/runtime boundary](spec/06-node-runtime-boundary.md)
7. [Scheduler V1](spec/07-scheduler-v1.md)
8. [Artifact/store protocol](spec/08-artifact-store-protocol.md)
9. [Repository/CMake structure](spec/09-repository-cmake-structure.md)
10. [Security model](spec/10-security-model.md)

## Decisions

All 46 accepted project decisions now have short ADR-style records. The canonical, ordered index is
[Decision records](decisions/README.md); a thematic view is the [Decision map](maps/decisions.md).
Each record links to the architecture/specification document where the decision applies.

## Open questions

The [open questions register](questions/README.md) tracks unresolved design and implementation
issues with status, source and next action. The [open-question map](maps/open-questions.md) groups
them by theme. Add new questions through [`templates/question.md`](templates/question.md).

## Glossary and log

- [Glossary](glossary.md) — shared terminology.
- [Implementation log](log/README.md) — session journal and current next actions.
- [References](references/README.md) — external standards and sources.
- [Archive](archive/README.md) — superseded material and archive policy.

## Conventions

- **MUST / SHOULD / MAY** are used in the RFC-2119 sense.
- Q-n identifies an answered design decision; every Q-n has a record in [`decisions/`](decisions/README.md).
- OI-n identifies an open question in [`questions/README.md`](questions/README.md).
- Component names: `pluribuscore`, `pluribusmaster`, `pluribusnode`, `pluribussim`,
  `pluribusfabric`, `pluribusstore`, CLI component `plucli` (binary `plu`).
- Canonical docs use standard relative Markdown links; frontmatter is metadata, not a second source
  of truth.

## Contributing to the knowledge base

1. Capture the thought in the [inbox](inbox.md) or an open question.
2. Create or update the smallest durable artifact using a [template](templates/note.md).
3. Link it from a map and from related notes.
4. Update the [implementation map](maps/implementation.md) and [log](log/README.md) when code or
   status changes.
5. If a choice was made, add a decision record and link it from the decision map.
6. If a note and a spec disagree, fix the note or change the spec deliberately.
