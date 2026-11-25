---
type: note
id: security
status: active
maturity: growing
tags: [note, security, trust]
aliases: ["Security model", "Trust boundary"]
sources: ["../spec/10-security-model.md", "../architecture/07-cross-cutting.md"]
last_reviewed: 2026-11-25
---

# Security

> V1 assumes trusted operators, trusted nodes/clients and a trusted development network. It is
> **not safe for untrusted code**: an execution runs with the node's OS privileges and can access the
> host resources available to that account.

## V1 controls

- Shared node join token, explicit listener bind, firewall and least-privilege account.
- Protocol version/role validation before application messages; fail closed.
- Bounded control frames (1 MiB total), bounded fields/counts/strings/nesting/buffering.
- Separate data-plane limits; no bulk data disguised as control.
- Persisted node UUID, separate from the mutable name; ambiguous names do not select silently.
- Path validation at every input/package/output boundary; reject traversal, absolute/drive/UNC
  paths, NUL, duplicates, symlinks/hardlinks/special entries in manifests; verify resolved paths.
- Clean, explicitly constructed environment; reserve `PLURIBUS_*`; no shell interpolation.
- SHA-256 + size verification before use; digest proves bytes, not author or ABI compatibility.

## Explicit non-guarantees

- A join token is not encryption, authentication of the master, client authorization or replay
  protection.
- The sandbox is not a security boundary.
- Process-tree stop/kill is best effort; descendants can escape.
- Same-user malicious processes are not isolated from the local service endpoint.
- A compromised master/client/node can compromise participating machines.

## Threat notes

A node executes arbitrary binaries by design, making the master effectively remote code execution
as a service. Bind to a LAN interface only and document it. Vehicle Wi-Fi and broader networks
should eventually be treated as untrusted.

## Future hardening

- TLS with authenticated certificates, authorization, credential lifecycle/rotation and replay
  resistance.
- Signed packages and provenance in addition to content hashes.
- Windows Job Objects/restricted tokens; Linux cgroups/namespaces/seccomp; macOS sandbox and
  privilege separation.
- Stronger per-execution IPC isolation, host/network policy, audit logs and quotas.

## Related

- [Sandbox](sandbox.md)
- [Fabric](fabric.md)
- [Artifact store](artifact-store.md)
- [Master](master.md)
- [Node](node.md)

## Canonical source

- [Security model](../spec/10-security-model.md)

## Open questions

- [OI-08 Authentication and client identity](../questions/README.md#oi-08-authentication-and-client-identity)
- [OI-23 Client authorization and local isolation](../questions/README.md#oi-23-client-authorization-and-local-isolation)
- [OI-24 Hard isolation threat model](../questions/README.md#oi-24-hard-isolation-threat-model)
