# 6. Store and packaging

## 6.1 `pluribusstore`

Content-addressed artifact storage and caching.

`ArtifactId = SHA-256(content)`

Instead of "send foo.hpp to node 3", the master says "execution needs artifact `A91F...`"; the node
checks its cache, reuses if present, fetches if absent. Automatic cross-execution caching.

Artifact: immutable hash, size, optional logical name, optional metadata.

Directories: later represented as **manifests** (sorted list of `path, mode, artifactId`), itself an
artifact. Benefits: dedup, caching, deterministic inputs, reproducibility, incremental sync.

## 6.2 Local cache

- Layout: `<cacheRoot>/sha256/ab/cdef...` (fan-out dirs), atomic write via temp + rename.
- Verify hash on fetch (and optionally on use).
- Decision (Q-36): LRU by configurable node/master capacity (20 GB is a development default, not a
  protocol constant); node config selects cache location. Artifacts needed by running executions are pinned.
- Cache summary (set/bloom filter or counts) reported in node dynamic state for locality scoring.

## 6.3 Transfer

- Source: client (upload on submit), master-side store, or peer node (Q-37).
- Over the data plane; chunked, resumable (later), flow-controlled.
- Decision (Q-37): Master indexes locations/metadata; peer-to-peer transfer is the long-term model,
  avoiding a mandatory master bottleneck. V1 may relay via master or a co-located node.

## 6.4 Inputs and outputs

- Inputs: materialized into the sandbox (`input/`, `work/`) by copy or hardlink/reflink where supported;
  read-only inputs may be shared via hardlink, rw inputs are copied.
- Outputs: declared paths in the sandbox are hashed and published as artifacts after exit; retained for a TTL; fetched via handle.
- No distributed POSIX filesystem in V1. `pluribusfs` (unified `/work`, `/packages`, `/artifacts`, `/tmp`)
  is a future layer requiring cache coherence, locking, consistency, metadata sync, failure recovery.

## 6.5 Packages (`.plp`)

The first end-to-end slice needs minimal package resolution and artifact materialization; richer
packaging follows. A `.plp` is itself an artifact (archive with manifest).

```
radarscape-sar-worker.plp
  manifest
  bin/pluribus-v1-x86_64/sar-worker
  bin/pluribus-v1-arm64/sar-worker
  resources/ (config, models, shaders, static data)
```

Manifest fields: name, version, Pluribus ABI version, supported platforms, entry points (named
executables with default args/exports/constraints), resources, required capabilities, minimum
resources, declared services.

Compatibility is determined automatically from manifest metadata: the scheduler picks a node for
which the package has a compatible executable.

Decision (Q-3, Q-4): Compatibility is Pluribus ABI plus CPU architecture, never host OS plus
architecture; V1 supports native binaries only. Prototype the ABI/runtime mechanism to validate
this contract across hosts. Scripts, WASM and managed runtimes may later be packaged as runtimes.

Other considerations: package signing (later), versioning/resolution (exact version pin supported),
dependency between packages (not in V1), shared libraries bundled per platform.

Decision (Q-5): `plu run hello` resolves a package; `./hello` uploads a local client file as an
ephemeral artifact/package; `artifact:sha256:...` selects an immutable artifact; `hello@1.4.2` selects
an exact package version. Node-local PATH lookup is not part of normal execution.

## 6.6 Reproducibility

Execution inputs are fully specified by hashes + args + env, enabling deterministic inputs and,
later, result caching (memoization) — not in V1 but the model must not preclude it.
