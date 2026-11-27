---
type: note
id: protocol-versioning
status: active
maturity: evergreen
tags: [note, protocol, versioning, abi]
aliases: ["Versioning", "Compatibility"]
sources: ["../spec/05-wire-protocol.md", "../spec/06-node-runtime-boundary.md", "../architecture/07-cross-cutting.md#77-versioning--compatibility"]
last_reviewed: 2026-11-25
---

# Protocol versioning

> Keep three contracts separate: wire protocol version, Pluribus ABI version, and package format
> version. None of them is host OS.

## Domains

| Contract | Meaning | Example |
|---|---|---|
| Wire protocol | Framing, message types, handshake, sequencing | major 1, bootstrap 0 |
| Pluribus ABI | Runtime/executable compatibility for native apps | `pluribus-v1` |
| Package format | `.plp` manifest/archive encoding | `format_version = 1` |

Scheduling compatibility is **ABI + CPU architecture** (e.g. `pluribus-v1-x86_64`).

## Wire rules

- Handshake negates a mutually supported major and optional features before application frames.
- Within a compatible major, add optional fields with defined defaults, preserve meanings, reserve
  removed identifiers and tolerate unknown optional fields.
- Never reuse an identifier with a different meaning.
- Unknown required features or flags fail negotiation, not silent semantic change.
- Breaking framing/meaning requires a new major.
- Enum extension and unknown-field preservation must be verified for the chosen codec.

## Sequence domains are independent

- Wire frame sequence (per direction, per session).
- Node lifecycle `event_seq` (per assignment).
- Master execution revision.
- Log cursor.
- Service registry revision.

Do not compare or persist them as if they were one number.

## Versioning status

The wire design is **not frozen**. Protocol Buffers is selected for V1 control payloads (Q-47), but
the schemas, numeric type registry and flags remain provisional. Publication of a stable version
requires conformance fixtures, negotiation/evolution tests, security review and a release/deprecation
policy.

## Related

- [Fabric](fabric.md)
- [Packages](package.md)
- [Runtime](runtime.md)
- [Execution lifecycle](execution-lifecycle.md)

## Canonical sources

- [Wire protocol](../spec/05-wire-protocol.md)
- [Runtime boundary: ABI, identity, configuration](../spec/06-node-runtime-boundary.md#abi-identity-and-configuration)
- [Cross-cutting: versioning](../architecture/07-cross-cutting.md#77-versioning--compatibility)

## Open questions

- [OI-16 Wire compatibility and conformance](../questions/README.md#oi-16-wire-compatibility-and-conformance)
- [OI-04 Portable native ABI](../questions/README.md#oi-04-portable-native-abi)
- [OI-06 Control message schema and type registry](../questions/README.md#oi-06-control-message-schema-and-type-registry)
