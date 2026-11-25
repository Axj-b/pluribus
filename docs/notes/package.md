---
type: note
id: package
status: active
maturity: growing
tags: [note, packages, abi, store]
aliases: ["Package", ".plp", "Package manifest"]
sources: ["../spec/08-artifact-store-protocol.md#package-model", "../architecture/06-store-and-packaging.md"]
last_reviewed: 2026-11-25
---

# Packages

> A `.plp` package is an immutable artifact containing a versioned manifest and native entries or
> references to blobs. Compatible placement is selected by **Pluribus ABI + CPU architecture**, not
> host OS.

## Shape

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

The exact archive/manifest encoding is not frozen. The snippet is a logical example, not a valid
wire contract.

## Resolution

- `plu run hello` resolves a package name; `hello@1.4.2` selects an exact version;
  `artifact:sha256:...` selects immutable content; `./hello` uploads a client-local ephemeral
  artifact/package.
- Resolve a mutable package name once on acceptance and record the selected immutable content.
- Do not use a node's `PATH` or infer portability from a filename.
- Select an entry from ABI + architecture; verify its content ID before launch.
- The scheduler filters by ABI/architecture keys derived from the package.

## Feasibility caveat

Native executable formats and host libraries differ across OSes. ABI/architecture keys are the
intended runtime contract, not proof that one native binary runs everywhere. A real portable
ABI/loader and packaging mechanism remain to be demonstrated. Reject unsupported execution rather
than claiming compatibility.

## Related

- [Artifact store](artifact-store.md)
- [Runtime](runtime.md)
- [Scheduler](scheduler.md)
- [Security](security.md)
- [CLI](cli.md)

## Canonical sources

- [Package model](../spec/08-artifact-store-protocol.md#package-model)
- [Store and packaging](../architecture/06-store-and-packaging.md)
- [Executable source syntax](../spec/01-execution-model.md#identity-and-request)

## Open questions

- [OI-04 Portable native ABI](../questions/README.md#oi-04-portable-native-abi)
- [OI-20 Cross-host ABI package mapping](../questions/README.md#oi-20-cross-host-abi-package-mapping)
- [OI-28 Minimal hello resolution](../questions/README.md#oi-28-minimal-hello-resolution)
