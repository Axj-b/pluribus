---
type: note
id: artifact-store
status: active
maturity: growing
tags: [note, store, artifacts, cache]
aliases: ["pluribusstore", "Artifact", "Content-addressed cache"]
sources: ["../spec/08-artifact-store-protocol.md", "../architecture/06-store-and-packaging.md"]
last_reviewed: 2026-11-25
---

# Artifact store

> Files cross Pluribus as immutable, content-addressed artifacts. The identity is
> `sha256:` plus 64 lower-case hex digits; it identifies bytes, not a filename, permission or author.

## Objects

- A store record is `{id, size, optional logicalName, optional metadata}`.
- Verify size and digest before publishing or using an object.
- An empty file is valid.
- A directory can be a manifest artifact: an ordered list of relative paths, types, modes and child
  blob IDs. Canonical encoding and sorted entries make equivalent descriptions hash identically.
- V1 rejects symlinks, device files and hardlinks in manifests.

## Node cache

- One configurable cache per node, e.g. `<cache-root>/sha256/ab/<62 hex>`.
- Write to a temporary area, verify the complete size/digest, then atomically publish on the same
  filesystem.
- Never expose a temporary or failed file as a cached object.
- Deduplicate concurrent fetches/publication.
- Evict least-recently-used **unpinned** objects under capacity pressure.
- Pin inputs through materialization/execution and outputs through publication/transfer.
- If pinned data and temporary writes do not fit, fail explicitly; never evict an active pin.

## Transfer

- The master tracks location hints; bytes use separate flow-controlled data connections.
- A requester asks for locations, opens a data connection, receives ordered bounded chunks, verifies
  the complete object and then publishes/announces it.
- Hash mismatch, truncation, excess bytes or failure discards the temporary object.
- V1 may relay via the master or a co-located node; no P2P requirement forces the master to carry
  every byte forever.

## Inputs and outputs

- Inputs are fetched missing IDs, verified, path-validated and materialized in the sandbox.
- Writable inputs are independent copies; a read-only hardlink/reflink is allowed only if the cache
  cannot be mutated through it.
- Declared outputs are collected after exit under the same confinement rules, hashed, published and
  associated with the execution.
- Output/log retention is master-configured; completion is not a permanent storage promise.

## Workspace cache idea

See [Workspace cache](workspace-cache.md) and
[the idea note](../../todo/content-addressed-workspace-cache.md). It is a proposal, not current
design.

## Related

- [Packages](package.md)
- [Workspace cache](workspace-cache.md)
- [Sandbox](sandbox.md)
- [Node](node.md)
- [Master](master.md)

## Canonical sources

- [Artifact store protocol](../spec/08-artifact-store-protocol.md)
- [Store and packaging](../architecture/06-store-and-packaging.md)

## Open questions

- [OI-19 Artifact manifest and package resolution](../questions/README.md#oi-19-artifact-manifest-and-package-resolution)
- [OI-15 Data-plane framing and limits](../questions/README.md#oi-15-data-plane-framing-and-limits)
- [OI-26 Workspace snapshot adoption](../questions/README.md#oi-26-workspace-snapshot-adoption)
