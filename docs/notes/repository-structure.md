---
type: note
id: repository-structure
status: active
maturity: growing
tags: [note, repository, cmake, dependencies]
aliases: ["Repository layout", "CMake structure", "Targets"]
sources: ["../spec/09-repository-cmake-structure.md", "../architecture/08-roadmap.md#repository-layout"]
last_reviewed: 2026-11-25
---

# Repository structure

> One repository until boundaries stabilize. Targets enforce the dependency direction; the public
> surface of each component is deliberately small.

## Layout

```text
CMakeLists.txt  CMakePresets.json
cmake/          shared compiler/build helpers
core/           identifiers, execution/resources/services, protocol contracts
fabric/         control/data framing and transport
store/          hashing, artifacts, manifests, cache
runtime/        pluribussim, sandbox, backend interface
platform/       win32/, linux/, macos/, driveos/
master/         registry, scheduler, execution manager, service/artifact index
node/           agent, executor, supervision, cache coordination
client/         public C++ client
cli/            plu executable
tests/          unit/, component/, integration/, system/, fixtures/
docs/           architecture/, spec/, second-brain layer
```

## Allowed dependencies

```text
core: none
fabric -> core
store -> core
runtime -> core
platform backend -> runtime, core
scheduler -> core
master -> core, fabric, store, scheduler
node -> core, fabric, store, runtime
client -> core, fabric
plu -> client
```

Forbidden: `core -> platform`, core -> host OS headers, `master -> RadarScape`,
`scheduler -> application domain`, `fabric -> RadarScape`, cyclic or transitive bypasses.

## Enforcement

- Target-scoped include directories, compile features and link visibility.
- A configure/CI check should enumerate the evaluated target graph and compare it to the allowlist,
  including `INTERFACE_LINK_LIBRARIES`, aliases and generator expressions.
- Scan source/includes (including transitive dependency files) for forbidden platform/application
  headers in core, scheduler and fabric.
- Compile portable core/scheduler tests with no platform include directories.
- Check installed public headers in a clean consumer build.
- Test the checks with deliberate negative fixtures.

## Toolchain

- C++23, CMake + Ninja, per-platform presets.
- Recommended (not yet adopted): vcpkg manifest mode with pinned baseline and triplets.
- Keep dependencies small, maintained and license-reviewed.
- Benchmark serialization and networking options rather than choosing by popularity.

## Current implementation

`pluribuscore`, `pluribusfabric` (framing) and `pluribusscheduler` exist with tests. The dependency
graph check and vcpkg baseline are still open.

## Related

- [Testing](testing.md)
- [Protocol versioning](protocol-versioning.md)
- [Implementation map](../maps/implementation.md)

## Canonical sources

- [Repository/CMake structure](../spec/09-repository-cmake-structure.md)
- [Roadmap and layout](../architecture/08-roadmap.md#repository-layout)

## Open questions

- [OI-21 License and platform validation](../questions/README.md#oi-21-license-and-platform-validation)
- [OI-22 Dependency enforcement and toolchain pinning](../questions/README.md#oi-22-dependency-enforcement-and-toolchain-pinning)
- [OI-27 Test and dependency baseline](../questions/README.md#oi-27-test-and-dependency-baseline)
