# 09. Repository and CMake structure

This is the intended implementation structure, not a claim that these targets or build presets
already exist. See [runtime boundary](06-node-runtime-boundary.md), [scheduler](07-scheduler-v1.md),
[store](08-artifact-store-protocol.md), [public C++ API](03-cpp-public-api.md), and
[wire protocol](05-wire-protocol.md).

## Layout

Keep one repository until boundaries stabilize:

```text
CMakeLists.txt  CMakePresets.json  vcpkg.json
cmake/          toolchains/, dependency checks, compiler options
core/           identifiers, execution/resource/service types, protocol contracts
fabric/         control/data framing and transport
store/          hashing, artifacts, manifests, cache
runtime/        pluribussim, sandbox and backend interface
platform/       win32/, linux/, macos/, driveos/ backend implementations
master/         registry, scheduler, execution manager, service and artifact index
node/           agent, executor, supervision and cache coordination
client/         C++23 public client library
cli/            plu executable
tests/          unit/, component/, integration/, system/, fixtures/
docs/           architecture/, spec/
```

Separate public `include/pluribus/` headers from private implementation includes within each
component. `pluribus::` is the C++ namespace; avoid leaking dependency or host types through
public headers. No stable C ABI is promised in V1. `plu` uses the same public client API as other
clients. Schema definitions are language-neutral, not C++ object layouts.

## Targets and allowed dependencies

Arrow means **links to/depends on**; only these Pluribus component edges are allowed:

| Target | Allowed Pluribus dependencies | Purpose |
|--------|-------------------------------|---------|
| `pluribuscore` | none | portable types/contracts |
| `pluribusfabric` | `pluribuscore` | messaging and transport |
| `pluribusstore` | `pluribuscore` | artifacts/cache; caller supplies transport |
| `pluribussim` | `pluribuscore` | runtime and abstract host backend |
| `pluribus_backend_<platform>` | `pluribussim`, `pluribuscore` | selected host implementation |
| `pluribusscheduler` | `pluribuscore` | pure scheduler, no I/O |
| `pluribusmaster` | `pluribuscore`, `pluribusfabric`, `pluribusstore`, `pluribusscheduler` | orchestration |
| `pluribusnode` | `pluribuscore`, `pluribusfabric`, `pluribusstore`, `pluribussim` | agent |
| `pluribusclient` | `pluribuscore`, `pluribusfabric` | public C++ client |
| `plu` | `pluribusclient` | CLI |

Composition root for the node executable links `pluribusnode` and exactly one selected backend;
the abstract runtime does not include platform headers. This dependency inversion avoids a
`pluribussim`/backend target cycle: the executable supplies a backend implementing the runtime
interface. Host/system libraries are private to the backend or appropriate transport implementation.
Executables can have separate internal library and entry-point targets where needed.

Forbidden: `core -> platform`; `core -> Windows/Linux/macOS` headers or libraries;
`master -> RadarScape`; `scheduler -> application domain`; `fabric -> RadarScape`. Also prevent
any Pluribus component depending on a client application, CMake-as-an-application domain, or
Thor-specific application logic. `driveos` is a host backend, not a domain dependency. Core has no
Pluribus dependencies; no cyclic edges or transitive bypasses are permitted.

Use target-scoped include directories, compile features and `target_link_libraries` with the
minimum `PRIVATE`/`PUBLIC` visibility; export only what an installed header actually needs.
For example:

```cmake
target_compile_features(pluribuscore PUBLIC cxx_std_23)
target_link_libraries(pluribusscheduler PRIVATE pluribuscore)
target_link_libraries(pluribusnode PRIVATE pluribussim pluribusfabric pluribusstore)
target_link_libraries(pluribus_node_app PRIVATE pluribusnode pluribus_backend_selected)
```

Where a public header uses another target's public headers, the corresponding interface link must
instead be `PUBLIC`; audit target interfaces rather than applying the example mechanically.

## Mechanical enforcement

At configure/CI time enumerate direct and transitive target link dependencies, including
`INTERFACE_LINK_LIBRARIES`, aliases and configuration-conditioned links, and compare them to an
allowlist. Fail on forbidden edges, cycles or unclassified internal targets; test the check with
deliberate negative fixtures. Use CMake file API or evaluated target graph so generator expressions
cannot hide a dependency. Independently scan source/includes, including transitive compiler
dependency files in CI, for forbidden platform and application headers in core, scheduler and
fabric. Compile portable core/scheduler tests with no platform include directories. Check installed
public headers in a clean consumer build. Maintain allowlists narrowly and review changes; no
single textual grep is a complete dependency proof.

## Dependencies and toolchains

Recommendation: **vcpkg manifest mode**, with a checked-in manifest, pinned baseline/registry,
documented triplets and pinned CI toolchain versions; cache binary packages by toolchain and
triplet. This is a proposed choice, to validate on DRIVE OS before adoption. Keep dependencies
small, maintained and license-reviewed. Use established libraries for SHA-256, networking and
serialization rather than custom cryptography/serialization. Benchmark FlatBuffers, Cap'n Proto
and Protocol Buffers using representative control messages before selecting a wire codec; likewise
measure networking options.

| Strategy | Advantages | Costs/risks |
|----------|------------|-------------|
| vcpkg manifest | CMake integration, reproducible baseline, cross-platform ports | DRIVE OS triplet/ports may need maintenance; binary cache and baseline discipline |
| Conan | flexible profiles, binary management and cross-compilation | more separate configuration/infrastructure and package recipe maintenance |
| FetchContent | simple for a few CMake-native sources, no package server | transitive reproducibility, offline builds, patching and supply-chain review become manual |

Avoid mixing unpinned system packages with locked dependencies in CI. A constrained toolchain may
need a documented custom triplet/sysroot or an approved alternative, not a silent fallback.

| Platform | V1 compiler candidates | Validation |
|----------|------------------------|------------|
| Windows | MSVC or Clang | C++23 compiler and standard library; Win32 backend |
| Linux | GCC or Clang | C++23 library and Linux backend |
| macOS | Apple Clang or Clang | C++23 library and macOS backend |
| DRIVE AGX Thor / DRIVE OS Linux | NVIDIA-supported DRIVE OS toolchain | actual SDK/sysroot, compiler and C++23 library support must be validated |

Pin exact tested versions in CI and toolchain configuration; do not claim DRIVE C++23 support
until tested. Use CMake + Ninja and define configure/build/test presets per supported platform,
debug/release, and cross-toolchain. Presets should inherit shared warnings, C++23 settings, and
dependency baseline; cross presets require explicit SDK/sysroot supplied by the operator. Example
names: `windows-msvc-debug`, `linux-clang-debug`, `macos-appleclang-debug`, `driveos-arm64-release`.
Do not encode machine-specific secret paths in committed presets.

## Tests and CI

Use unit tests for IDs, scheduling, manifests, cache and protocol validation; component tests use
fake backend/transport; integration tests start real master/node processes; system tests run two
nodes over loopback and later real multi-host binaries. Include failure/partition, stream
backpressure and security/path cases. `ctest --preset <preset>` should run the corresponding
registered tests after `cmake --preset <preset>` and `cmake --build --preset <preset>` once presets
exist. CI runs enforcement, unit/component and loopback tests on supported hosts; cross builds and
hardware tests run when the DRIVE SDK/hardware are available. No build/test success is implied by
this design document.

## License and attribution

Pluribus should be free to use, including in other projects, with attribution/reference to the
project/author. Evaluate standard MIT, BSD-3-Clause and Apache-2.0 licenses and their notice,
patent and attribution terms before publication; do not invent a custom license or claim one is
already chosen. Audit all third-party dependencies and distribute their required notices.

## Open issues

- Select and publish the license; validate vcpkg and C++23 with the actual DRIVE OS SDK.
- Pin compiler, dependency and preset versions and implement graph/include CI checks when the
  first build targets are added.
