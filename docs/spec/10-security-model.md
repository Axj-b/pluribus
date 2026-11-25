---
title: "10. Security model"
type: spec
id: spec-10
status: draft
tags: [spec, security]
last_reviewed: 2026-11-25
---

# 10. Security model

## Boundary and assumptions

V1 is for trusted operators, nodes, clients, native programs and a trusted development network.
It is **not safe for untrusted code**: a native execution runs with the node's OS privileges and
can access host files, processes, network and hardware permitted to that account. The execution
directory is an operational workspace, not a security sandbox. Pluribus provides remote execution
by design; compromise of a permitted master/client/node can compromise participating machines.
Vehicle Wi-Fi and broader networks must not be presumed trusted.

Security controls below prevent common mistakes and corruption; they do not establish protection
against hostile network peers or hostile executions. See [execution model](01-execution-model.md),
[node/master protocol](02-node-master-protocol.md), [public API](03-cpp-public-api.md),
[service model](04-service-model.md), [wire protocol](05-wire-protocol.md),
[runtime](06-node-runtime-boundary.md), and [store](08-artifact-store-protocol.md).

## Threat model

| Threat / failure | V1 measure | Residual risk |
|------------------|------------|---------------|
| Accidental node joins | Shared node join token and protocol handshake | Plain TCP exposes token; replay, theft and spoofing remain possible |
| Network exposure / unauthorized client | Explicit listener bind interface, firewall and trusted network/operator | No TLS or strong peer authorization; do not expose to untrusted networks |
| Malformed or oversized messages | Version validation, bounded frames, field/count/size limits | Parser bugs and resource exhaustion remain possible |
| Artifact corruption / wrong bytes | Verify expected SHA-256 and length before publication/use | Hash is not signature, trust in metadata/source still required |
| Traversal via input, output or archive entries | Validate logical paths and safely resolve beneath sandbox for runtime-managed operations | Does not confine a running hostile program or defeat all host-level races by itself |
| Unintended host environment leakage | Construct a clean environment with explicit variables and bounded runtime bridges | Required native paths and local endpoint details may be visible to the execution |
| Identity confusion / stale node | Persist node UUID separately from name; reconcile identity and execution records | UUID is an identifier, not cryptographic proof of identity |
| Spoofed service registration | Bind local endpoint operations to execution context as supported | No robust isolation against a malicious same-user process in V1 |
| Process tree escape / runaway resources | Best-effort supervision, accounting and explicit stop/kill | No security isolation or hard resource enforcement; descendants can escape |
| Master/client disconnect | Preserve process and reconcile; do not equate transport loss with termination | Orphan policy and bounded logs can leave uncertainty or data loss |

## Admission, binding and protocol

Configure the master and other externally reachable listeners with an explicit bind interface and
port; do not silently listen on all interfaces. Restrict network reachability with OS firewall
rules and run under a dedicated least-privilege account. Nodes use static master configuration
and a shared join token read from an operator-protected secret file or equivalent; do not include
the secret in source control, command-line logs or diagnostic events. Compare tokens safely,
reject missing/invalid tokens before accepting registration, and avoid returning secrets in errors.
Define the node handshake and rejection through [02](02-node-master-protocol.md) and
[05](05-wire-protocol.md). A join token is a basic admission check, **not** confidential or a
substitute for TLS, authentication of the master, client authorization, or replay protection.
Limit token distribution and rotate it operationally if exposed.

Every connection validates protocol version and role before accepting application messages;
unsupported/incompatible versions fail closed. Enforce the V1 control-frame maximum of 1 MiB
before allocation, plus bounded collection counts, strings, nesting and per-peer buffering.
Set separate configured data-plane frame/chunk, object and concurrent-transfer limits; do not
route bulk artifacts through control frames. Check lengths for overflow, reject malformed or
truncated messages, close invalid connections and log a bounded diagnostic without secrets.
Transport sequence/retry handling follows [05](05-wire-protocol.md).

Node identity is a persisted UUID, with a separate configurable human-readable name. Name changes
do not change identity and ambiguous names do not select a node silently. Duplicate UUID claims,
unexpected concurrent sessions or conflicting reconciliation evidence must be rejected or handled
by an explicit master policy; do not silently replace an active node. The UUID is not a credential.
Client connection lifetime is independent of execution lifetime; disconnection alone is not a
security cancellation signal.

## Paths, environment and artifacts

At every input, package extraction and output collection boundary, validate relative logical paths
before filesystem access. Reject absolute paths, empty or dot/dot-dot components, traversal using
either separator, drive/UNC prefixes, embedded NUL, duplicate destinations and platform-specific
aliases that would escape or collide. Reject symlinks, hardlinks and special entries in V1 artifact
manifests. Check that resolved destinations stay under the intended sandbox subtree, including
when a path component is already a symlink, junction or reparse point. Use handle-relative,
no-follow operations where available, verify during creation/open rather than only string-prefix
checking, and fail closed if safe confinement for the operation cannot be established. Do not
follow application-created links while collecting outputs. Mitigate time-of-check/time-of-use races
with backend-specific safe operations; the check confines Pluribus-managed file operations, not
the process itself. Enforce file count and byte limits during materialization/collection.

Start the process with a clean, explicitly constructed environment: required runtime variables plus
the request's allowed values, not the master's or node's full inherited environment. Reserve the
`PLURIBUS_*` namespace for runtime-controlled values and reject conflicting application overrides.
The V1 path variables may contain native paths; treat them as transitional process-local values,
not persistent public identities. Avoid unintentionally passing credentials, search paths or loader
settings. Execute the selected file directly with arguments, never interpolate them into a shell.

SHA-256 and size are checked against the requested artifact ID before an object is made available
or executed, including uploads and outputs. A mismatch or truncation fails the operation and removes
the incomplete temporary data; never advertise it as cached. Verify package manifest references
and validate extraction paths independently of the hash. A digest proves byte identity, not author
identity, safety or that a native binary is ABI-compatible. Signed packages are future work.

The runtime-provided local language-neutral endpoint is scoped to the execution as far as the host
permits, and it accepts only operations associated with that execution. Protect endpoint handles,
permissions and any per-execution credential from accidental exposure. This is not a guarantee
against malicious processes running as the same user; use the [service model](04-service-model.md)
for readiness, registration and byte-stream semantics.

## Future hardening

- TLS with authenticated certificates for master, nodes and clients; authenticated authorization,
  credential lifecycle, rotation and replay resistance.
- Signed packages and provenance verification in addition to content hashes.
- Windows Job Objects and restricted tokens; Linux cgroups, namespaces and seccomp; macOS sandbox
  facilities and privilege separation. These require explicit design, platform testing and audit;
  resource accounting and best-effort process groups are not equivalent protections.
- Stronger per-execution local IPC isolation, host/network policy, audit logs, quotas and defense
  against a compromised node or master.

## Open issues

- Specify client authorization, token provisioning/rotation, exact data-plane limits and safe
  per-platform filesystem/IPC mechanisms as implementation and protocol choices mature.
- Define and validate a hard isolation threat model before permitting untrusted workloads or
  claiming the sandbox is a security boundary.

## Knowledge base links

- Map: [Specification map](../maps/specifications.md)
- Notes: [Security](../notes/security.md), [Sandbox](../notes/sandbox.md)
- Decisions: [Decision records](../decisions/README.md), [Decision map](../maps/decisions.md)
- Open questions: [Open questions register](../questions/README.md)
