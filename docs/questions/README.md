---
type: index
id: open-questions
status: active
tags: [question, register, guidance]
aliases: ["Open questions", "Open issues"]
last_reviewed: 2026-11-25
---

# Open questions register

> The canonical register of unresolved design and implementation questions. Each entry now explains
> the problem, suggests a behavior, records the trade-offs and names the next action. The theme view
> is in the [open-question map](../maps/open-questions.md).

## How to read a suggestion

- A **suggested behavior** is a proposal for discussion, not an accepted decision.
- **Confidence** is how safe the proposal looks with the evidence available today: high, medium or low.
- If a suggestion is accepted, create a decision record in [`decisions/`](../decisions/README.md),
  update the affected architecture/spec text and mark the OI `resolved`.
- A suggestion can be rejected or superseded; keep the entry for history.
- Suggestions never override the canonical specs. When they conflict, the spec wins until it is
  deliberately changed.

## OI-01 ExecutionId representation

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues), [Core model](../architecture/02-core-model.md#21-strong-identifiers)
- **Question:** How is an execution identified on the wire and in storage, and what is the friendly CLI ID?
- **Explanation:** An execution identity must be globally unambiguous across master restarts, reconnects and stored reports. A short ID is useful for humans but is not safe as a wire identity: prefixes can collide and a lookup can become ambiguous. The core already models UUID-based IDs, but the on-wire representation, short-ID length and lookup rules are not frozen.
- **Suggested behavior (proposal):** Use a 128-bit UUIDv4 as the canonical `ExecutionId`; send and store the 16-byte value or its lower-case canonical text, never a short ID. Keep `RequestId` separate. For CLI/display, show the first 8–12 hex characters and accept a unique prefix; reject ambiguous prefixes with `AMBIGUOUS`. Do not let short IDs appear in protocol messages or durable references.
- **Trade-offs:** Short prefixes are convenient but require a master-side index and collision handling. Twelve characters is safer than eight for large execution counts; eight is acceptable if ambiguity is always rejected.
- **Confidence:** high
- **Next action:** Use this rule when the first master execution index is implemented; add short-ID lookup tests.
- **Related:** [Q-10](../decisions/execution.md#q-10), [Execution lifecycle](../notes/execution-lifecycle.md)

## OI-02 Owner identity and lifetime

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues), [Public API](../spec/03-cpp-public-api.md#open-issues)
- **Question:** Who owns an execution, and what happens when the submitting client disconnects or detaches?
- **Explanation:** Execution lifetime is deliberately independent of a TCP connection, but some clients may want owner-lifetime coupling. That requires a stable client identity and a policy that is not confused with socket liveness. In V1 there is no strong client authentication, so an owner identity is advisory unless it is also used as a deduplication scope.
- **Suggested behavior (proposal):** Give each client a persisted `ClientId` UUID. Default `owner_lifetime` to `independent`: disconnecting or detaching never stops the execution. Support `owner_lifetime=client` only as an explicit opt-in with a configured grace period after the last owner session disappears; never infer termination from a socket close. For the first vertical slice, implement only the independent policy and reject `client` until client identity and dedup exist.
- **Trade-offs:** Owner-lifetime coupling adds orphan and notification complexity. Without authentication, a client can claim another owner’s ID; treat ownership as coordination metadata, not authorization.
- **Confidence:** medium
- **Next action:** Define the client config/identity format and the grace-period policy before implementing `--detach`.
- **Related:** [Q-21](../decisions/execution.md#q-21), [Master](../notes/master.md)

## OI-03 Orphan policy and retention

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues), [Runtime boundary](../spec/06-node-runtime-boundary.md#disconnect-reconciliation-and-services)
- **Question:** What should the master and node do with executions whose ownership cannot be proven after a restart or reconnect?
- **Explanation:** A master restart or a conflicting node report can leave processes running without a trusted execution record. Killing them can destroy useful work; adopting them blindly can attach work to the wrong identity. Logs, outcomes and sandboxes also cannot be retained forever.
- **Suggested behavior (proposal):** Default to `orphan_policy=hold`: keep the process running locally, do not expose its services, accept no new globally scheduled work, and require an explicit reconciliation/operator action. Allow `orphan_policy=stop` as a configuration. Adopt only when `NodeId`, node incarnation, `ExecutionId` and `AssignmentId` all match and the node supplies credible state; otherwise keep `Lost`. Suggested retention defaults: 10 MiB of log history per execution, terminal outcome records 7 days, successful sandboxes 1 hour, failed sandboxes 24 hours, and full retention while persistent executions are active. Make all of these configurable.
- **Trade-offs:** `hold` consumes resources and can leave users waiting; automatic stopping is easier to operate but can kill legitimate work. Retention defaults are guesses until real workloads are measured.
- **Confidence:** high
- **Next action:** Add the policy keys and reconciliation tests with the first master/node implementation.
- **Related:** [Q-13](../decisions/execution.md#q-13), [Node](../notes/node.md)

## OI-04 Portable native ABI

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues), [Runtime boundary](../spec/06-node-runtime-boundary.md#open-issues), [Store](../spec/08-artifact-store-protocol.md#open-issues)
- **Question:** What exactly does the Pluribus ABI promise, and how can a native binary be known to run on a given node?
- **Explanation:** CPU architecture and a version string are not enough. Native executable formats, system libraries and OS services differ even on the same ISA. The documents deliberately keep host OS out of the scheduler, but the runtime contract still has to encode enough information to prevent false compatibility. A true OS-neutral ABI or loader has not been demonstrated.
- **Suggested behavior (proposal):** Treat the ABI key as the **complete runtime-contract identifier**, not a vague product version. For V1, use explicit keys such as `pluribus-v1-win32-x86_64` and `pluribus-v1-linux-x86_64` (and corresponding arm64 keys), and make package variants use the same keys. The scheduler still filters on ABI + architecture; it does not gain a separate host-OS field. Prototype a genuinely portable ABI/loader separately and only then introduce an OS-neutral key. Reject unknown or unproven variants rather than hoping they run.
- **Trade-offs:** This multiplies package variants and may look like host OS leaking into the ABI. It is honest: the alternative is claiming portability that does not exist. A future portable runtime can collapse the keys without changing scheduler semantics.
- **Confidence:** medium
- **Next action:** Write the ABI-key grammar and prototype a two-OS `hello` package before freezing package metadata.
- **Related:** [Q-3](../decisions/store-packaging.md#q-3), [Q-4](../decisions/store-packaging.md#q-4), [Package](../notes/package.md), [OI-20](#oi-20-cross-host-abi-package-mapping)

## OI-05 Exit status and output collection

- **Status:** open
- **Source:** [Execution model](../spec/01-execution-model.md#open-issues)
- **Question:** How are process exit, signal, collection failure and missing outputs represented in the final execution outcome?
- **Explanation:** An execution can exit normally, exit non-zero, be killed by a signal, or produce an unknown outcome after a partition. Separately, output collection can fail or be incomplete even when the process exited. Mixing these into one boolean loses information and makes reports irreproducible.
- **Suggested behavior (proposal):** Model the outcome as `{ termination: exited | signaled | unknown, exit_code: optional<int32>, signal: optional<int32>, collection: ok | partial | failed, collection_error: optional<Error> }`. Process exit determines `Completed` (zero) or `Failed` (non-zero/abnormal); collection failure is reported separately and does **not** silently rewrite a known exit. Missing declared outputs are a collection error. If no credible exit evidence exists, the execution remains `Lost`.
- **Trade-offs:** Consumers must inspect collection status, not just the state. Some operators may prefer any collection failure to fail the execution; make that a master policy later, not a hidden default.
- **Confidence:** high
- **Next action:** Add the outcome fields to the core model and spec; add terminal-state tests for each combination.
- **Related:** [Execution lifecycle](../notes/execution-lifecycle.md), [Artifact store](../notes/artifact-store.md)

## OI-06 Control message schema and type registry

- **Status:** open
- **Source:** [Node/master protocol](../spec/02-node-master-protocol.md#open-issues), [Wire protocol](../spec/05-wire-protocol.md#open-issues)
- **Question:** What payload schema, numeric type IDs, flags and bootstrap encoding define the control protocol?
- **Explanation:** Framing is implemented, but a frame is only useful once its payload is specified. The protocol needs stable numeric assignments for negotiation and dispatch, a way to correlate requests and responses, and a policy for unknown fields and flags. Choosing these ad hoc would make evolution expensive.
- **Suggested behavior (proposal):** Define one versioned envelope with `request_id`, optional portable `Error`, and a `oneof` payload. Reserve numeric ranges: `0` invalid, `1–3` bootstrap, `16–63` control-plane, `64–127` services/artifacts, `128–255` extensions; publish the registry in one source of truth. Keep flags `0` in V1, with a documented future bit range. Unknown optional fields are preserved; unknown required flags/versions fail negotiation. Choose the codec via [OI-14](#oi-14-serializer-selection) before freezing payloads.
- **Trade-offs:** Reserving ranges is cheap and prevents later collisions; a premature concrete schema could lock in mistakes. Keep bootstrap messages tiny and separately versioned from application messages.
- **Confidence:** medium
- **Progress (2026-11-27):** the bootstrap handshake uses a versioned Protobuf schema. The numeric `MessageType` registry, `Envelope` (with `request_id`, portable `Error` and body `oneof`) and `RegisterNode`/`RegisterNodeAck`/`Heartbeat`/`HeartbeatAck` bodies are defined in `control.proto` and implemented by a bounded codec. The frame `type` byte is the sole dispatch key; bootstrap messages are not wrapped.
- **Next action:** define the remaining bodies (types 20–79) and their OI dependencies, then add role/direction enforcement and session dispatch; freeze only after conformance fixtures.
- **Related:** [Fabric](../notes/fabric.md), [Wire protocol](../spec/05-wire-protocol.md)

## OI-07 Endpoint tickets and route authorization

- **Status:** open
- **Source:** [Node/master protocol](../spec/02-node-master-protocol.md#open-issues), [Service model](../spec/04-service-model.md#open-issues)
- **Question:** How does a resolver obtain a connection to a service without learning a durable host address?
- **Explanation:** Service resolution should return an opaque recipe, not an IP address and port. That recipe must identify the provider generation, the intended consumer and the purpose, and must not become a long-lived credential. A resolve can also race with provider removal, so failure must be explicit.
- **Suggested behavior (proposal):** The master issues a short-lived opaque ticket (for example 128 random bits) bound to provider ID/generation, owning execution/assignment, node, purpose (`service-connect`) and an expiry of about 30 seconds. The ticket is single-use or limited-use and is presented on a separate data connection; the node validates it before connecting to the local endpoint. No durable host address is returned. On failure, re-resolve. V1 may broker through the master; direct node-to-node routing can come later.
- **Trade-offs:** Tickets add master work and a replay window. They are routing capabilities, not encryption or strong authorization; the trusted-network assumption still applies.
- **Confidence:** medium
- **Next action:** Define the ticket fields, expiry and validation errors with the first service registry.
- **Related:** [Services](../notes/service.md), [Q-30](../decisions/fabric-services.md#q-30)

## OI-08 Authentication and client identity

- **Status:** open
- **Source:** [Node/master protocol](../spec/02-node-master-protocol.md#open-issues), [Wire protocol](../spec/05-wire-protocol.md#open-issues), [Security](../spec/10-security-model.md#open-issues)
- **Question:** How are nodes and clients admitted, identified and authorized in V1?
- **Explanation:** A shared join token admits a node to the master, but it is not encryption, client identity or replay protection. The master still needs a stable identity for request deduplication, ownership and audit. V1 assumes a trusted development network, so the goal is accident prevention and clear boundaries, not protection from a hostile peer.
- **Suggested behavior (proposal):** Keep the V1 stance explicit: trusted network, best-effort admission. Nodes use a join token from an operator-protected file, compared in constant time, with explicit bind and no secrets in logs. Clients use a persisted `ClientId` UUID plus an optional shared client token; if no client token is configured, mark identity as advisory and do not expose the master to untrusted networks. Scope deduplication to `ClientId`. Defer TLS, certificates, rotation and replay protection to a security milestone.
- **Trade-offs:** Client IDs are spoofable on a hostile network and tokens can be replayed. The documentation must not imply otherwise. Strong auth is a larger design and should not be half-implemented.
- **Confidence:** medium
- **Next action:** Implement constant-time token comparison and the client config; add a threat note to the security spec.
- **Related:** [Q-41](../decisions/engineering.md#q-41), [Security](../notes/security.md)

## OI-09 Request deduplication and durable master state

- **Status:** open
- **Source:** [Node/master protocol](../spec/02-node-master-protocol.md#open-issues), [Wire protocol](../spec/05-wire-protocol.md#open-issues)
- **Question:** How does a retried submission avoid creating a second execution, and what survives a master restart?
- **Explanation:** A client that loses a response cannot know whether the master accepted the request. Retry safety requires a stable request key and a canonical request fingerprint. V1 keeps master state in memory, so deduplication cannot survive a restart; the protocol must say that clearly instead of implying exactly-once execution.
- **Suggested behavior (proposal):** Keep an in-memory map `(ClientId, RequestId) -> {canonical_fingerprint, ExecutionId, accepted_at}`. The fingerprint is a SHA-256 over a canonical encoding of the immutable spec, excluding the request ID (which is the key). Same fingerprint returns the original `ExecutionId`; a different fingerprint under the same key returns `CONFLICT`. Retention defaults to 24 hours or a configured window. On master restart, the map is lost; clients must query by `ExecutionId` or accept that a resubmission may duplicate work. Add durable storage later behind a WAL/transaction boundary.
- **Trade-offs:** Canonicalization is easy to get wrong; document exactly which fields participate. In-memory retention is simple but weak after a crash.
- **Confidence:** high
- **Next action:** Define the canonical fingerprint and dedup tests with the first master.
- **Related:** [Q-20](../decisions/control-plane.md#q-20), [Q-28](../decisions/fabric-services.md#q-28), [Master](../notes/master.md)

## OI-10 Public API ownership and async model

- **Status:** open
- **Source:** [Public API](../spec/03-cpp-public-api.md#open-issues)
- **Question:** How are handles owned, copied and used across threads, and what does async cancellation mean?
- **Explanation:** A handle must be cheap to copy but must not duplicate the execution. Destroying a handle or disconnecting must not stop work. Async operations need an executor, a lifetime and a cancellation model that does not leak third-party I/O types into the public API. Exceptions are not the primary operational error channel.
- **Suggested behavior (proposal):** Make `ExecutionHandle` a copyable value that shares library-owned state; destruction never terminates the execution. Use `Result<T>`/`std::expected` for fallible operations and a library-owned `task<T>` for async forms. Cancelling a wait or log stream stops only the local operation, never the execution; `terminate`/`kill` are explicit. Handle operations are thread-safe. Keep the public headers free of native handles, PIDs, sockets and third-party async types. Implement the synchronous surface first, then async.
- **Trade-offs:** A library-owned executor is convenient but adds lifecycle complexity. Blocking-only V1 would simplify, but the architecture explicitly targets async event-driven I/O.
- **Confidence:** medium
- **Next action:** Freeze the sync surface with a fake client and write async cancellation tests before publishing the API.
- **Related:** [Q-6](../decisions/engineering.md#q-6), [CLI](../notes/cli.md)

## OI-11 Log cursor and stream semantics

- **Status:** open
- **Source:** [Public API](../spec/03-cpp-public-api.md#open-issues)
- **Question:** How do consumers resume logs, detect gaps and distinguish EOF from an error?
- **Explanation:** Stdout and stderr are separate ordered streams with bounded retained history. A subscriber can start after eviction, reconnect after a gap, or reach the end of a live stream. If cursor, gap and shutdown semantics are vague, consumers silently lose data or hang.
- **Suggested behavior (proposal):** Give each stream a monotonic `sequence` and a cursor containing `{stream, sequence}`. A chunk carries `{stream, sequence, timestamp, bytes}`. `logs(stream, after?, follow)` replays retained history then live data; if `after` is older than retention, return an explicit `gap` and start at the oldest available chunk. EOF means the selected streams reached a terminal execution state; transport errors are separate. Keep chunk size bounded (for example 64 KiB) and never interleave stdout/stderr as a total order.
- **Trade-offs:** Sequence-based cursors are simple but require the node to retain sequence metadata with the bytes. Byte-offset cursors are smaller but ambiguous across restarts.
- **Confidence:** high
- **Next action:** Define the cursor/chunk types with the node log history implementation.
- **Related:** [Q-12](../decisions/execution.md#q-12), [Node](../notes/node.md)

## OI-12 Service-name grammar

- **Status:** open
- **Source:** [Service model](../spec/04-service-model.md#open-issues)
- **Question:** What exact characters, length and normalization rules define a service name?
- **Explanation:** Service names are logical identifiers, not hostnames or paths, and must interoperate across implementations. The hierarchy is for organization, not authorization or wildcard routing. Without one grammar, two clients can disagree about whether `Foo/Bar`, `foo/bar` or `foo//bar` are the same name.
- **Suggested behavior (proposal):** Freeze a lower-case ASCII grammar: slash-separated segments where each segment matches `[a-z0-9]` followed by an optional `(?:[a-z0-9._-]{0,61}[a-z0-9])?`; total length 1–255; at most 16 segments; no empty segment, `.` or `..`; no host, port, scheme or wildcard. Reject non-canonical names instead of normalizing them, so the stored name is exactly the requested name. Update the provisional `ServiceName` parser to match.
- **Trade-offs:** Strictness rejects existing mixed-case experiments and may feel unfriendly. Case-folding would be more forgiving but creates ambiguity and cross-platform surprises.
- **Confidence:** high
- **Next action:** Align the core parser and tests with the proposed grammar before service registration crosses implementations.
- **Related:** [Services](../notes/service.md), [Q-30](../decisions/fabric-services.md#q-30)

## OI-13 Duplicate provider and readiness policy

- **Status:** open
- **Source:** [Service model](../spec/04-service-model.md#open-issues)
- **Question:** What happens when two providers register the same logical name, and how is readiness maintained?
- **Explanation:** The model permits multiple providers, but V1 resolution returns at most one ready provider. Without a deterministic conflict policy, resolution depends on registration order and stale entries. Readiness is an application signal, not proof of future health, and node loss must not silently keep advertising a service.
- **Suggested behavior (proposal):** In V1, allow at most one active **ready** provider per name. Reject a second active registration with `CONFLICT`; keep the protocol able to represent multiple providers for the future. Use a monotonic provider generation so stale readiness cannot win. On node unavailability, unpublish the provider but retain evidence for reconciliation; on reconnect, revalidate identity and readiness before exposing it again. Drain stops new resolutions while existing streams may continue. Do not add active health probing in V1 beyond explicit readiness + node reachability.
- **Trade-offs:** Rejecting duplicates is deterministic but prevents accidental failover/load sharing. A future policy can add selection or multiple-provider resolution without changing identity.
- **Confidence:** high
- **Next action:** Implement the registry conflict/readiness tests with the first service registry.
- **Related:** [Q-31](../decisions/fabric-services.md#q-31), [Q-33](../decisions/fabric-services.md#q-33), [Services](../notes/service.md)

## OI-14 Serializer selection

- **Status:** resolved
- **Source:** [Wire protocol](../spec/05-wire-protocol.md#serialization-benchmark-and-selection), [Q-15](../decisions/fabric-services.md#q-15)
- **Question:** Which wire codec should carry control messages?
- **Explanation:** The codec affects encoded size, decode latency, allocations, schema evolution, tooling and cross-language support. The spec explicitly requires a benchmark rather than choosing by popularity. Data-plane throughput must not dominate a control-message decision.
- **Suggested behavior (proposal):** Benchmark FlatBuffers, Cap'n Proto and Protocol Buffers on the documented message mix and select one codec for all control traffic. Provisional default: Protocol Buffers, because its schema evolution, tooling and cross-language support are strong; switch only if the benchmark shows a decisive advantage for another option on bounded decode/latency/allocations. Pin generated-code versions and test unknown-field behavior before freezing payloads.
- **Trade-offs:** Protobuf adds generated code and copies; FlatBuffers/Cap'n Proto can reduce decode cost but add schema/tooling constraints. A single codec keeps the protocol simpler than mixing.
- **Confidence:** high
- **Resolution:** Protocol Buffers selected for V1 control payloads in [Q-47](../decisions/fabric-services.md#q-47). See the [codec benchmark report](../reports/005-codec-benchmark.md).
- **Next action:** Define the versioned Protobuf control schemas, generate code, and replace the provisional bootstrap codec.
- **Related:** [Fabric](../notes/fabric.md), [Protocol versioning](../notes/protocol-versioning.md)

## OI-15 Data-plane framing and limits

- **Status:** open
- **Source:** [Wire protocol](../spec/05-wire-protocol.md#open-issues), [Store](../spec/08-artifact-store-protocol.md#open-issues), [Security](../spec/10-security-model.md#open-issues)
- **Question:** How are artifacts, logs and service bytes framed and bounded on the data plane?
- **Explanation:** Bulk traffic must not block heartbeats or termination, and a peer must not be able to force unbounded allocation. Data connections also need to identify their purpose and be tied to an authorized control-plane operation. The exact chunk/object limits are still open.
- **Suggested behavior (proposal):** Open a data connection with a purpose ticket; negotiate the purpose and limits; then send ordered length-prefixed chunks with an offset. Use a bounded chunk size (for example 1 MiB maximum, 64 KiB typical), an object-size limit (for example 20 GB development default), and at most four concurrent transfers per node. Apply flow control per connection. V1 transfers a complete object from offset zero and verifies SHA-256 before publishing; resumption is deferred. Never tunnel bulk bytes through control frames.
- **Trade-offs:** Smaller chunks bound memory but add overhead; larger objects need streaming and disk staging. No resume is simpler but restarts a failed transfer from zero.
- **Confidence:** high
- **Next action:** Add the data-frame format, purpose ticket and limits to the wire/store specs before implementing transfer.
- **Related:** [Artifact store](../notes/artifact-store.md), [Fabric](../notes/fabric.md)

## OI-16 Wire compatibility and conformance

- **Status:** open
- **Source:** [Wire protocol](../spec/05-wire-protocol.md#compatibility-and-stable-contract)
- **Question:** When can the wire protocol be called stable, and how is evolution tested?
- **Explanation:** A schema can look forward-compatible and still fail on unknown fields, enum growth or a removed identifier. The protocol is not frozen today; declaring stability without fixtures and old/new peer tests would create a false contract.
- **Suggested behavior (proposal):** Keep wire major 1 explicitly experimental until a conformance suite exists. Add checked-in golden frames for Hello/Welcome/Reject, RegisterNode, Heartbeat, Submit, Assign, StateEvent, ServiceReady/Resolve and Error. Test old/new peers in both directions, unknown optional fields, unknown required flags, malformed/oversized frames and identifier reuse. Reserve removed IDs permanently. Publish the release/deprecation policy with the suite; a breaking framing or semantic change requires a new major.
- **Trade-offs:** Waiting for a suite delays a “stable” label but avoids accidental lock-in. Some early adopters may need to tolerate experimental changes.
- **Confidence:** high
- **Next action:** Create `tests/wire/fixtures` and the first compatibility tests once the codec is chosen.
- **Related:** [Protocol versioning](../notes/protocol-versioning.md)

## OI-17 Host backend signatures and environment

- **Status:** open
- **Source:** [Runtime boundary](../spec/06-node-runtime-boundary.md#open-issues)
- **Question:** What exactly does the `HostBackend` interface expose, and which environment variables may an execution see?
- **Explanation:** The backend is the boundary that keeps host handles and OS headers out of the rest of the platform. It needs concrete ownership, error, async and process-tree semantics. The `PLURIBUS_*` environment bridge is useful but can leak physical paths and must not become a public identity. Log-gap and retention behavior also need a concrete shape.
- **Suggested behavior (proposal):** Define `HostBackend` around `Result<T>`, opaque `ProcessHandle`, `ProcessSpec` (argv, explicit env, cwd, no shell), separate stdout/stderr pipes, process-tree terminate/kill, `ProcessStatus`, `ProcessUsage`, bounded file operations, pipes, local endpoints and host resources. Make stream reads/writes async or pollable rather than exposing native sockets. Pass only an explicit clean environment plus `PLURIBUS_INPUT`, `PLURIBUS_WORK`, `PLURIBUS_OUTPUT`, `PLURIBUS_TMP` and `PLURIBUS_SERVICE_ENDPOINT`; reject application overrides of `PLURIBUS_*`. Treat those values as transitional native paths, never persistent identifiers. Implement a Win32 fake backend first for deterministic tests.
- **Trade-offs:** A rich interface is harder to port but prevents platform leakage later. A tiny spawn-only interface would be faster to build but would leak host assumptions into node/runtime.
- **Confidence:** medium
- **Next action:** Write the backend header and a fake implementation with component tests before selecting a real backend.
- **Related:** [Runtime](../notes/runtime.md), [Sandbox](../notes/sandbox.md), [Q-23](../decisions/execution.md#q-23)

## OI-18 Scheduler defaults and measurement freshness

- **Status:** open
- **Source:** [Scheduler](../spec/07-scheduler-v1.md#open-issues)
- **Question:** Which default reservations and freshness windows should V1 use?
- **Explanation:** The scheduler policy is implemented, but deployment defaults are configuration, not protocol constants. Too-small defaults overcommit nodes; too-large defaults leave work queued. Stale measurements are dangerous because the policy deliberately double-counts measured usage and reservations.
- **Suggested behavior (proposal):** Adopt development defaults of 1000 millicores CPU and 1 GiB memory per execution, a 5-second maximum measurement age, a 1-second heartbeat and a 5-missed-heartbeat unavailability threshold. Use the configured local node only as an exact-score tie-break. Keep cache, historical profiles and labels out of V1 ordering. Validate the conservative accounting against real build/test workloads and make every default configurable.
- **Trade-offs:** Conservative accounting can delay work unnecessarily, especially with overlapping measurements. Aggressive defaults can overload a node and make latency worse.
- **Confidence:** high
- **Next action:** Add the policy keys to master config and exercise them in loopback tests with fake node capacities.
- **Related:** [Scheduler](../notes/scheduler.md), [Q-16](../decisions/control-plane.md#q-16)

## OI-19 Artifact manifest and package resolution

- **Status:** open
- **Source:** [Store](../spec/08-artifact-store-protocol.md#open-issues)
- **Question:** How are directory manifests, packages and leases encoded and resolved?
- **Explanation:** A manifest's identity is the hash of its exact bytes, so the encoding must be canonical and deterministic. Package resolution needs an index from a mutable name/version to immutable content. Cache leases and retention determine when bytes can be evicted. These are coupled: an ambiguous encoding or resolution rule produces silently different package contents.
- **Suggested behavior (proposal):** Use a canonical JSON manifest (RFC 8785-style sorted keys and normalized numbers) with sorted relative paths, entry types, modes, sizes and blob IDs; reject symlinks, hardlinks, device files and unsafe paths. A package is a manifest artifact plus referenced blobs. The master maintains a `name -> version -> manifest artifact` index; exact versions resolve immutably, and a mutable name is resolved once on acceptance. Use configurable leases for active inputs/outputs. V1 may relay bytes through the master; peer transfer comes later. Do not add workspace snapshots to this first schema; see [OI-26](#oi-26-workspace-snapshot-adoption).
- **Trade-offs:** JSON is debuggable but larger and needs a rigorous canonicalizer. A binary format is smaller but harder to inspect. Any canonicalization bug changes artifact identity.
- **Confidence:** medium
- **Next action:** Define the canonical manifest schema and a golden-encoding test before implementing the store.
- **Related:** [Artifact store](../notes/artifact-store.md), [Package](../notes/package.md)

## OI-20 Cross-host ABI package mapping

- **Status:** open
- **Source:** [Store](../spec/08-artifact-store-protocol.md#open-issues)
- **Question:** How does one package select the right executable for a node without adding host OS as a scheduling dimension?
- **Explanation:** This is the packaging half of [OI-04](#oi-04-portable-native-abi). A package may contain several native variants, but the scheduler only sees ABI + architecture keys. If the variant key is too coarse, the wrong binary is selected; if the selection falls back across OSes, the result is a failed launch or worse.
- **Suggested behavior (proposal):** Make the package variant key exactly the runtime-contract ABI + architecture from OI-04. The master selects an exact key, never a fallback. If no variant matches, the execution is `UNSCHEDULABLE` with a clear reason. Validate the binary format and content hash before launch. If a truly portable ABI appears later, add it as another key; do not add a host-OS scheduling field.
- **Trade-offs:** Multi-OS packages carry more variants and larger transfer sizes. Exact selection is safer than optimistic fallback.
- **Confidence:** high
- **Next action:** Prototype a two-OS `hello` package and test selection/rejection on both hosts.
- **Related:** [Package](../notes/package.md), [OI-04](#oi-04-portable-native-abi)

## OI-21 License and platform validation

- **Status:** open
- **Source:** [Repository](../spec/09-repository-cmake-structure.md#license-and-attribution)
- **Question:** Which license should Pluribus use, and does the DRIVE OS toolchain actually support the chosen C++23 baseline and dependencies?
- **Explanation:** Publication requires a standard free license with attribution, and third-party notices must be compatible. The DRIVE OS target is a feasibility question, not a documentation checkbox: compiler, standard library, CMake, vcpkg and networking code all need validation on the real SDK/sysroot.
- **Suggested behavior (proposal):** Prefer Apache-2.0 for its patent grant and explicit attribution/notice terms; if minimal terms are more important to the project, choose MIT or BSD-3-Clause instead. Add `LICENSE` and a third-party notice process before publication. On DRIVE OS, compile a small C++23 probe with the official toolchain, validate the standard library features used by core/scheduler, and try vcpkg with a pinned custom triplet or documented sysroot. Do not claim support until the probe and a `hello` build run on the target.
- **Trade-offs:** Apache-2.0 is more explicit but longer; MIT/BSD-3 are simpler. vcpkg on DRIVE OS may require maintenance; an approved alternative is better than an unpinned system package.
- **Confidence:** medium
- **Next action:** Make the license decision and add a CI/target probe checklist.
- **Related:** [Q-8](../decisions/scope.md#q-8), [Q-38](../decisions/engineering.md#q-38)

## OI-22 Dependency enforcement and toolchain pinning

- **Status:** open
- **Source:** [Repository](../spec/09-repository-cmake-structure.md#mechanical-enforcement)
- **Question:** How do we prove that the documented dependency direction is actually enforced?
- **Explanation:** A diagram does not prevent a `core -> platform` include or a hidden transitive link. CMake target links, interface libraries, aliases and generator expressions can all hide an edge. Source-level includes can bypass target boundaries entirely. Without a check, the architecture degrades silently.
- **Suggested behavior (proposal):** Add a parameterless `scripts/check-deps.ps1` (or an equivalent CMake/CI check) that evaluates the target graph, compares it to the allowlist in the spec, rejects cycles and unclassified internal targets, and scans source/includes for forbidden platform/application headers. Test the checker with deliberate negative fixtures. Pin compiler, preset and dependency versions in CI. Run it before adding `pluribusmaster` and `pluribusnode`, while the graph is still small.
- **Trade-offs:** The checker needs maintenance and can produce false positives; those are preferable to silent boundary violations. Graph evaluation must not rely on one textual grep.
- **Confidence:** high
- **Progress (2026-11-27):** a configure-time CMake graph check and a `scripts/check-deps.ps1` include scan are implemented and passing; negative fixtures and toolchain pinning are still open.
- **Next action:** Add negative fixtures, transitive include scanning in CI, and pin compiler/dependency versions.
- **Related:** [Q-39](../decisions/engineering.md#q-39), [Q-43](../decisions/engineering.md#q-43), [Repository structure](../notes/repository-structure.md)

## OI-23 Client authorization and local isolation

- **Status:** open
- **Source:** [Security](../spec/10-security-model.md#open-issues)
- **Question:** What stops a local or remote client from using another execution’s endpoint or services?
- **Explanation:** V1 has no TLS or strong peer authorization. The local service endpoint is scoped to an execution “as far as the host permits”, but a same-user process can often reach it. The goal is accident prevention and clear residual risk, not a security boundary.
- **Suggested behavior (proposal):** Keep V1 explicitly trusted-network. Use a per-execution random endpoint secret, OS ACLs/permissions where available, and a local-only bind; bind service operations to the execution identity. Do not expose the local endpoint on a broad interface. Document that a malicious same-user process can bypass this. Defer TLS, client certificates and per-execution OS isolation to the security milestone.
- **Trade-offs:** Per-execution secrets add provisioning complexity but reduce accidental cross-talk. They do not stop a determined same-user attacker.
- **Confidence:** medium
- **Next action:** Define the local endpoint authentication fields and test same-user/other-user behavior on Windows and Linux.
- **Related:** [Security](../notes/security.md), [Q-41](../decisions/engineering.md#q-41)

## OI-24 Hard isolation threat model

- **Status:** open
- **Source:** [Security](../spec/10-security-model.md#open-issues)
- **Question:** What must be true before Pluribus can run untrusted code?
- **Explanation:** A native execution currently runs with the node’s OS privileges and can read host files, network and hardware allowed to that account. The sandbox is an operational workspace, not a confinement mechanism. Claiming otherwise would be a serious safety and trust problem.
- **Suggested behavior (proposal):** Do not accept untrusted workloads in V1. Keep the documentation and CLI explicit that the sandbox is not a security boundary. Before making any isolation claim, write a threat model and implement OS-backed confinement: Windows Job Objects/restricted tokens, Linux namespaces/cgroups/seccomp, macOS sandbox/privilege separation, plus resource limits, audit and a compromised-node assumption. Treat this as a separate security milestone with tests and review.
- **Trade-offs:** Restricting to trusted code limits adoption but avoids false safety claims. Real isolation is platform-specific and expensive.
- **Confidence:** high
- **Next action:** Create a security-milestone proposal; do not expose an “untrusted mode” before it exists.
- **Related:** [Sandbox](../notes/sandbox.md), [Security](../notes/security.md)

## OI-25 Network stack selection

- **Status:** open
- **Source:** [Cross-cutting](../architecture/07-cross-cutting.md#71-technology-choices-v1), [Wire protocol](../spec/05-wire-protocol.md#serialization-benchmark-and-selection)
- **Question:** Which async networking stack should implement TCP transport?
- **Explanation:** The fabric needs bounded reads/writes, independent control/data connections, cancellation and backpressure. The stack affects coroutine integration, portability, build dependencies and latency. Choosing before measurement risks either a heavyweight dependency or a hand-rolled event loop that ages poorly.
- **Suggested behavior (proposal):** Keep a narrow transport/`Stream` interface and implement a small blocking TCP transport first to prove the handshake. Then benchmark a standalone Asio-based implementation (or Boost.Asio if already accepted) against platform sockets for loopback/LAN latency and allocations. Choose the async stack based on the documented targets, keep it private to the transport layer, and never expose its types in the public API.
- **Trade-offs:** A blocking-first spike delays async work but reduces protocol risk. Asio is portable and well maintained but adds a dependency and learning curve.
- **Confidence:** medium
- **Progress (2026-11-25):** the narrow blocking `Stream`, in-memory loopback and IPv4 TCP transport are implemented with loopback benchmarks. Async stack selection remains open.
- **Progress (2026-11-27):** a standalone Asio single-connection benchmark was slower than the blocking reference on WSL loopback (RTT p50 81.0 µs vs 63.6 µs, throughput 828 vs 5147 MiB/s). Blocking remains the reference; concurrency testing is still needed.
- **Next action:** Add a 32–128 concurrent-client benchmark for Asio and a comparable blocking reference, then decide the V1 stack.
- **Related:** [Q-40](../decisions/engineering.md#q-40), [Fabric](../notes/fabric.md)

## OI-26 Workspace snapshot adoption

- **Status:** proposed
- **Source:** [Workspace cache idea](../../todo/content-addressed-workspace-cache.md), [Store](../spec/08-artifact-store-protocol.md)
- **Question:** Should Pluribus adopt workspace trees/snapshots and missing-blob negotiation?
- **Explanation:** Repeated builds/tests send many unchanged files, SDKs and toolchains to workers. A workspace manifest of content hashes could transfer only missing blobs and reuse outputs. It also adds tree/snapshot metadata, invalidation, pinning and cache-correctness complexity. The scheduler must still schedule an execution referencing a snapshot, not interpret individual files.
- **Suggested behavior (proposal):** Defer adoption until the basic artifact store works. Then add an immutable tree manifest (sorted paths -> blob IDs), a snapshot root hash, a missing-hash query before execution, and the same pin/lease rules as blobs. Keep snapshots as a store concern; the scheduler sees only the execution and its snapshot reference. Measure transfer volume and cache hit rate before adding deltas or a distributed filesystem.
- **Trade-offs:** High value for repeated C++ builds, but a large correctness surface. Byte-level deltas and POSIX semantics are explicitly out of scope.
- **Confidence:** medium
- **Next action:** Update the store spec with a workspace-snapshot design proposal after artifact V1, not before.
- **Related:** [Workspace cache note](../notes/workspace-cache.md), [Artifact store](../notes/artifact-store.md)

## OI-27 Test and dependency baseline

- **Status:** open
- **Source:** [Repository](../spec/09-repository-cmake-structure.md#dependencies-and-toolchains), [Cross-cutting](../architecture/07-cross-cutting.md#75-testing-strategy)
- **Question:** Which dependency manager, test framework and hashing library should the project standardize on?
- **Explanation:** The current unit tests use a dependency-free harness and the project links no third-party library. That is useful for bootstrapping but does not scale to richer assertions, fixtures, parameterized tests or a maintained SHA-256 implementation. The spec recommends vcpkg manifest mode and GoogleTest, but neither is adopted and DRIVE OS support is unproven.
- **Suggested behavior (proposal):** Adopt a pinned vcpkg manifest with `gtest` for tests and a maintained SHA-256 library (OpenSSL or a smaller reviewed alternative) for artifacts; keep CTest as the driver and replace or wrap the temporary harness. Cache binary packages by toolchain/triplet and pin the baseline in CI. Validate the vcpkg triplet on DRIVE OS; if it is not viable, document an approved custom triplet/sysroot rather than silently using unpinned system packages.
- **Trade-offs:** vcpkg improves reproducibility but adds bootstrap and binary-cache work. GoogleTest is familiar but heavier than the current harness. A custom crypto implementation is not acceptable.
- **Confidence:** high
- **Next action:** Add `vcpkg.json`/baseline and migrate the tests once the first dependency-heavy target is needed.
- **Related:** [Testing](../notes/testing.md), [Repository structure](../notes/repository-structure.md)

## OI-28 Minimal hello resolution

- **Status:** open
- **Source:** [Store](../spec/08-artifact-store-protocol.md#submission-inputs-and-outputs), [Roadmap](../architecture/08-roadmap.md#milestones)
- **Question:** What is the smallest executable/package resolution path that can demonstrate `plu run hello`?
- **Explanation:** The first vertical slice needs the CLI to submit `hello`, the master to resolve it to immutable content, and the node to materialize the right ABI/architecture variant. Building a full package registry before the first execution would delay the architecture test. The resolver must still avoid implicit `PATH` lookup and must record the resolved content.
- **Suggested behavior (proposal):** For the first slice, use a master-side static package index from configuration: `hello -> package manifest artifact + variant keys`. Resolve `hello` once on acceptance to an immutable manifest and ABI/architecture variant, verify the hash, and record it in the execution. Keep `hello@version`, remote package publication and a general registry out of scope. Use `./hello` as a client-upload smoke test for the same path. This satisfies `plu run hello` without building a package service first.
- **Trade-offs:** A static config index is not a product registry, but it exercises the real resolution/materialization path. A general registry can replace it without changing execution semantics.
- **Confidence:** high
- **Next action:** Define the minimal manifest fields and static index shape with the first master/node vertical slice.
- **Related:** [Package](../notes/package.md), [Implementation map](../maps/implementation.md)

## Adding a question

Use [`templates/question.md`](../templates/question.md). Give it the next `OI-*` number, a source,
a question, an explanation, a suggested behavior, trade-offs and a concrete next action. Then add it
to the theme groups in [`maps/open-questions.md`](../maps/open-questions.md). If the suggestion is
accepted, create a decision record and update the affected spec.
