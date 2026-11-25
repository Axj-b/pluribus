# Pluribus Content-Addressed Workspace Cache

## Goal

Reduce network traffic between the master and worker nodes by keeping reusable copies of project
files on every node. For each execution, transfer only content that the selected worker does not
already have.

## Core idea

Each file is stored as an immutable blob identified by a cryptographic content hash, such as
SHA-256. A worker maintains a persistent local content-addressed store containing previously
transferred files, toolchains, dependencies, and generated artifacts.

Before starting a job, the master sends a workspace manifest rather than sending the complete
workspace. The manifest maps file paths to blob hashes and represents an immutable snapshot of the
workspace.

Example:

    CMakeLists.txt    -> hash A
    src/main.cpp      -> hash B
    src/radar.cpp     -> hash C
    include/radar.hpp -> hash D

The worker compares the hashes in the manifest with its local store and reports which blobs are
missing. The master transfers only those missing blobs. The worker then materializes the requested
workspace snapshot locally and starts the command.

If only `src/radar.cpp` changed, its hash changes while the hashes of all unchanged files remain
identical. Only the new `radar.cpp` blob needs to be transferred.

## Main concepts

| Concept | Meaning |
|---------|---------|
| Blob | Immutable file content identified by its hash |
| Tree | Immutable mapping of names to blobs or child trees |
| Snapshot | The root tree hash representing the complete workspace at a specific state |
| Node cache | A persistent local content-addressed store on every worker |
| Execution | A command, arguments, environment, and reference to a workspace snapshot |
| Transfer protocol | Exchange of manifests, available-hash checks, and missing-blob transfer |

## Outputs

Execution outputs should use the same mechanism. Compiled objects, generated files, packages, and
test results can be stored as hashed blobs. The master or another worker can reuse or retrieve these
artifacts without transferring or rebuilding content that already exists.

## Initial scope

Do not implement byte-level or rsync-style binary deltas initially. For typical C++ source and
header files, transferring the complete changed file is already cheap. The important optimization
is avoiding retransmission of unchanged files, SDKs, dependencies, toolchains, and build artifacts.

## Recommended first implementation

1. Calculate SHA-256 hashes for workspace files.
2. Build an immutable workspace manifest.
3. Maintain a persistent blob store on every worker.
4. Let the worker report missing hashes before execution.
5. Transfer only missing blobs.
6. Materialize the workspace in an isolated execution directory.
7. Store output artifacts as hashed blobs.
8. Add cache size limits and least-recently-used cleanup.
9. Verify every received blob against its expected hash.
10. Measure transfer volume and cache hit rate before adding binary delta encoding.

## Architectural rule

The scheduler should not interpret individual files. It schedules an execution referencing a
snapshot. Workspace synchronization and blob transfer are separate infrastructure services.

## Result

Pluribus moves content only when the selected node does not already have it. This should reduce
network load more effectively than replacing Gigabit Ethernet, especially when repeatedly building
and testing the same project across multiple nodes.

## Relation to existing docs

This idea extends `pluribusstore` and the artifact model described in
`docs/architecture/06-store-and-packaging.md` and `docs/spec/08-artifact-store-protocol.md`, adding
workspace trees/snapshots and explicit missing-blob negotiation before execution. When adopting it,
update those documents rather than treating this note as committed design.
