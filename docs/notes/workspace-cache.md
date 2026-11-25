---
type: note
id: workspace-cache
status: active
maturity: seed
tags: [note, store, workspace, proposal]
aliases: ["Content-addressed workspace cache", "Workspace snapshot"]
sources: ["../../todo/content-addressed-workspace-cache.md", "../spec/08-artifact-store-protocol.md"]
last_reviewed: 2026-11-25
---

# Workspace cache (proposal)

> Instead of sending a whole workspace to a worker, send a manifest of content hashes and transfer
> only blobs the worker does not already have.

## Idea

- Each file is stored as an immutable blob identified by SHA-256.
- A tree maps names to blobs or child trees; a snapshot is the root tree hash.
- Before a job, the master sends the snapshot manifest.
- The worker reports missing hashes; only those blobs are transferred.
- The worker materializes the snapshot in an isolated execution directory.
- Outputs use the same mechanism, enabling reuse of build artifacts.

## Why it might matter

For repeated C++ builds/tests across nodes, most files, SDKs, toolchains and dependencies are
unchanged. Avoiding retransmission may reduce network load more effectively than upgrading Gigabit
Ethernet.

## Status

This is a **proposal**, not committed design. It extends the artifact model with workspace
trees/snapshots and explicit missing-blob negotiation. Adopting it requires updating the artifact
store specification rather than treating the idea note as canonical.

## Non-goals for the first version

- Byte-level or rsync-style binary deltas.
- A distributed POSIX filesystem.
- Letting the scheduler interpret individual files; it schedules an execution referencing a
  snapshot.

## Related

- [Artifact store](artifact-store.md)
- [Packages](package.md)
- [Node](node.md)
- [Open questions register: OI-26](../questions/README.md#oi-26-workspace-snapshot-adoption)

## Canonical sources

- [Idea note](../../todo/content-addressed-workspace-cache.md)
- [Artifact store protocol](../spec/08-artifact-store-protocol.md)
- [Store and packaging](../architecture/06-store-and-packaging.md)
