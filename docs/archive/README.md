---
type: index
id: archive
status: active
tags: [archive, history]
last_reviewed: 2026-11-25
---

# Archive

> History is useful, but it should not compete with current knowledge. Archive superseded notes,
> decisions and docs here instead of deleting them.

## Policy

Archive a page when:

- a decision is superseded by a newer record;
- an architecture/spec section is replaced and the old text still explains history;
- a note is no longer useful but links point to it;
- an experiment is abandoned.

Do not archive:

- accepted decisions that are still in force;
- open questions (close them with a resolution link instead);
- current specs or architecture docs.

## How to archive

1. Move the file to `docs/archive/<original-path>` or add `status: archived` and a banner.
2. Add `supersedes` / `superseded_by` links.
3. Update every map and note that linked to it.
4. Add an entry to the implementation log or decision record explaining why.

## Current archive

- Empty. Nothing has been superseded yet.
