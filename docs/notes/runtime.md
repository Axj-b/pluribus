---
type: note
id: runtime
status: active
maturity: growing
tags: [note, runtime, abi, backend]
aliases: ["pluribussim", "Host backend", "Runtime boundary"]
sources: ["../spec/06-node-runtime-boundary.md", "../architecture/04-node-and-runtime.md"]
last_reviewed: 2026-11-25
---

# Runtime

> `pluribussim` is the uniform execution environment. It hides host-OS differences behind a small
> host-backend interface; it is **not** a Linux simulator.

## Responsibilities

- Sandbox setup and logical-path mapping.
- Clean, explicitly constructed environment.
- Process spawn/supervision and process-tree stop/kill.
- Separate timestamped stdout/stderr capture.
- Local service endpoint integration.
- Host resource/accelerator measurement through the backend.

## Host backend

The backend interface covers spawn, stop, kill, query, measure, directories, file I/O, pipes,
local endpoints and host resources. Backend objects encapsulate native handles and errors; public
types expose Pluribus errors, not PIDs, OS handles or native paths. Implementations are selected at
composition time: `win32`, `linux`, `macos`, later `driveos`.

## Important boundary

- V1 does **not** implement Linux syscall emulation, a virtual filesystem, CPU emulation or a
  general network stack.
- A Windows process cannot open a conceptual Unix path; logical paths are a runtime description, not
  a mount.
- V1 may pass `PLURIBUS_INPUT`, `PLURIBUS_WORK`, `PLURIBUS_OUTPUT`, `PLURIBUS_TMP` and a local
  service endpoint as native values. These are transitional bridges, not public identities.
- Compatibility is Pluribus ABI + CPU architecture. Native binaries are not assumed portable across
  host OSes merely because the architecture matches.
- The actual portable ABI/loader mechanism remains to be demonstrated.

## Related

- [Node](node.md)
- [Sandbox](sandbox.md)
- [Packages](package.md)
- [Protocol versioning](protocol-versioning.md)

## Canonical sources

- [Node/runtime boundary](../spec/06-node-runtime-boundary.md)
- [Node and runtime](../architecture/04-node-and-runtime.md)

## Open questions

- [OI-04 Portable native ABI](../questions/README.md#oi-04-portable-native-abi)
- [OI-17 Host backend signatures and environment](../questions/README.md#oi-17-host-backend-signatures-and-environment)
