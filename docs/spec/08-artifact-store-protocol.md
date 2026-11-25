# 08. Artifact store and transfer

Status: V1 contract and illustrative records; concrete serialization is governed by the
[wire protocol](05-wire-protocol.md), not these examples. See [execution model](01-execution-model.md),
[node/master protocol](02-node-master-protocol.md), [runtime](06-node-runtime-boundary.md),
[scheduler](07-scheduler-v1.md), and [security](10-security-model.md).

## Objects and manifests

An `ArtifactId` is `sha256:` followed by 64 lowercase hexadecimal digits, the SHA-256 digest of
the exact artifact bytes. IDs identify immutable content, not a filename, permissions or source.
Store records carry `{id, size, optional logicalName, optional metadata}`; verify size and digest
before use. Untrusted metadata is never part of identity unless encoded in an artifact's bytes.
An empty file is a valid artifact. Use a maintained hashing library.

Inputs and outputs can be individual blobs with a separate mapping from sandbox-relative path
to artifact ID and executable/mode metadata. A directory can be described by a manifest artifact:
an ordered list of relative paths, types, modes and child blob IDs. Its ID hashes its exact encoded
bytes; deterministic serialization and sorted entries are required when building manifests so
equivalent logical descriptions produce the same ID. Reject duplicate or unsafe paths and
unsupported entry types; V1 need not provide a general archive or distributed filesystem. In
particular, symlinks, device files and hardlinks in manifests are not accepted in V1.

## Node cache

One configurable node cache uses, for example, `<cache-root>/sha256/ab/<remaining-62-hex>` for
verified objects and a separate temporary area for in-progress transfers. Never expose a temp
file as an object: write, verify complete size and digest, then atomically publish on the same
filesystem. A crash or failed verification leaves no valid entry; remove abandoned temporary data
on recovery. Synchronize concurrent fetches/publication so identical blobs are safely deduplicated.

Capacity is configurable at the node, with master/operator policy as applicable; 20 GB is a
development default, not a wire constant. Evict least-recently-used **unpinned** objects when
space is needed, tracking access/recency in local metadata. Pin input objects before use, including
through materialization and execution; pin outputs during publication/transfer and while an
explicit retention lease requires them. Use reference counts/leases or equivalent so overlapping
executions cannot unpin each other's data. Release after the final consumer/lease ends. If pinned
data and temporary writes leave insufficient space, fail the fetch/materialization explicitly;
never evict an active pin or claim success. Startup must rebuild or reconcile pins before eviction.

Cache contents and location announcements are not durable storage guarantees. Materialize writable
inputs as independent copies. A read-only hardlink/reflink is permissible only if the cache cannot
be mutated through the materialized copy; otherwise copy. Treat an input file as immutable even
when a host's filesystem permissions cannot provide a security boundary.

## Data-plane transfer

The master tracks availability metadata; artifact bytes use separate, flow-controlled data-plane
connections, not bounded control messages. Logical exchanges below are not final message names or
a replacement for [05](05-wire-protocol.md):

1. A requester asks the master for locations of an ID and expected size. A location is a hint, not
   proof of current availability. V1 can use a master-side or co-located-node relay as the source;
   long term a client uploads to a nearby cache and nodes fetch directly from peers selected using
   the index. No future P2P assumption requires the master to carry every byte.
2. The requester opens a data connection and sends an artifact request for ID and expected size.
   The source responds with acceptance and length or a typed unavailable/error result. Apply
   authentication/authorization available for V1 and wire limits; the join token is not encryption.
3. Source streams ordered, bounded chunks with offsets and length. Enforce configured maximum
   object and chunk sizes before allocation; use backpressure independent of control traffic.
   Each transfer consumes the complete object in V1, starting at offset zero.
4. Requester checks order, length, declared size and SHA-256 across the complete stream, then
   atomically publishes the cache entry and announces the location. Hash mismatch, truncated stream,
   excess bytes or failure discards the temporary object and does not announce it. Retry from zero,
   optionally choosing another source. Resuming partial transfers is deferred; a future resume
   protocol must verify the complete resulting object before publication.

The index maps artifact IDs to verified size/metadata and currently advertised node/relay locations.
Locations are added only after verification, removed on eviction, and invalidated or marked
unavailable when a node disconnects or loses cache state. On reconnect the node reconciles its
advertisements; failed fetches update stale hints. Node loss is not proof every copy was deleted.
Transfer and index updates are retry-safe; duplicate announcements do not multiply locations.
Actual availability may lag the index, so requesters must handle misses.

## Submission, inputs and outputs

`plu run hello` resolves a package; `plu run hello@1.4.2` selects the exact package version;
`plu run artifact:sha256:...` identifies immutable content. For `plu run ./hello`, the client reads
the explicitly named local file, hashes and uploads it to the V1 relay/cache, and submits an
ephemeral artifact/package reference with explicit ABI and architecture metadata. It must not
interpret `hello` using a remote node's `PATH` or silently infer portability from a file name.
Submission references the verified ID, not a client-local path; transfer failure fails submission
or materialization explicitly. Client disconnect after accepted submission does not cancel the
execution, so the input remains pinned/retained for its required lifecycle.

The node fetches missing input IDs, verifies them, validates their relative destinations and
materializes them in the sandbox before spawn. Declared output paths are collected under the same
confinement rules after exit, hashed, published as artifacts and associated with the execution;
the client obtains IDs and fetches bytes separately. Missing/unreadable/unsafe outputs are
reported as collection errors according to the execution contract rather than inventing artifacts.
Outputs and logs have master-configured retention; no permanent storage promise follows from
execution completion. Refer to [01](01-execution-model.md) for terminal state and collection
semantics.

## Package model

A `.plp` is an immutable package artifact containing a versioned manifest and native executable
entries/resources, or equivalent references to immutable blobs. The following is an illustrative
logical manifest, not an already ratified archive encoding:

```toml
format_version = 1
name = "hello"
version = "1.4.2"

[variants.pluribus-v1-x86_64]
entry = "bin/hello-x86_64"
artifact = "sha256:<64-hex-digest>"

[variants.pluribus-v1-arm64]
entry = "bin/hello-arm64"
artifact = "sha256:<64-hex-digest>"
```

The `<64-hex-digest>` values illustrate fields, not literal valid IDs. Resolve an entry's key from
Pluribus ABI plus CPU architecture; check its content ID before launch. The package may also
declare resources, entry points and mandatory capabilities. Do not select by host OS as an
application scheduling capability. **Important feasibility limitation:** native executable formats
and host libraries differ across OSes; these keys are the intended Pluribus runtime contract, not
proof that a native binary can run unchanged on all hosts. A real portable ABI/loader and its
packaging mechanism remain to be demonstrated; reject unsupported execution rather than claim
compatibility. No ISA emulation, scripts, WASM or general runtime integration is assumed in V1.

## Garbage collection

Master retention policy governs execution/output references and leases; node LRU reclaims unpinned
local bytes under capacity pressure. Expiring a reference permits eventual eviction, not immediate
or global deletion. A cache eviction removes its advertised location; do not delete objects held
by other active references. A client/relay must keep a needed upload available until another
verified copy is pinned or the request has failed. No globally durable distributed garbage
collector or replication guarantee is implied by V1.

## Open issues

- Finalize canonical manifest/archive encoding, package registration/resolution, limits, transfer
  message schemas, and retention/lease durations alongside wire protocol implementation.
- Prototype the cross-host native ABI and how a single ABI/architecture package key maps to
  executable bits without introducing host OS as a scheduling dimension.
