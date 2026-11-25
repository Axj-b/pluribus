---
type: reference
id: references
status: active
tags: [reference, standards, external]
last_reviewed: 2026-11-25
---

# References

> External sources and standards that the design depends on. Add a link and one line explaining why
> it matters; do not paste large external documents here.

## Standards

| Reference | Why it matters |
|---|---|
| [RFC 2119](https://www.rfc-editor.org/rfc/rfc2119) | MUST / SHOULD / MAY vocabulary used in specifications |
| [RFC 4122](https://www.rfc-editor.org/rfc/rfc4122) | UUID format for node and execution identities |
| [FIPS 180-4](https://csrc.nist.gov/pubs/fips/180-4/upd1/final) | SHA-256 definition for artifact identity |
| [RFC 9293](https://www.rfc-editor.org/rfc/rfc9293) | TCP, the V1 transport |
| [Semantic Versioning](https://semver.org/) | Package version conventions (not yet a hard contract) |
| [CMake](https://cmake.org/documentation/) | Build, presets, target graph and file API |
| [C++23](https://en.cppreference.com/w/cpp/23) | Language baseline |
| [Protocol Buffers](https://protobuf.dev/), [FlatBuffers](https://flatbuffers.dev/), [Cap'n Proto](https://capnproto.org/) | Serializer candidates to benchmark |
| [vcpkg](https://vcpkg.io/) | Recommended (not adopted) dependency strategy |

## Internal sources

- [Original idea](../../IDEA.md)
- [Architecture](../architecture/01-overview.md)
- [Specifications](../spec/01-execution-model.md)
- [Workspace cache idea](../../todo/content-addressed-workspace-cache.md)

## How to add a reference

- Add it under the right section.
- State why it matters to Pluribus, not just what it is.
- Prefer stable canonical URLs.
- If a reference becomes obsolete, move it to the [archive](../archive/README.md) with a note.
