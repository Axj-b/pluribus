# Pluribus

Pluribus is a portable distributed execution environment that presents a set of
heterogeneous machines as one logical machine. Applications submit executions
and refer to logical services; they never select a physical node.

The design is recorded in:

- [`docs/README.md`](docs/README.md) — documentation home and second-brain dashboard
- [`docs/knowledge-base.md`](docs/knowledge-base.md) — how the linked docs system works
- [`docs/architecture/`](docs/architecture/01-overview.md) — architecture
- [`docs/spec/`](docs/spec/01-execution-model.md) — specifications
- [`IDEA.md`](IDEA.md) — original idea

## Current implementation

This repository is at the beginning of the implementation roadmap. The first
slice is in place:

| Target | Contents |
|---|---|
| `pluribuscore` | Strong IDs, portable errors, execution spec/state machine, node and service descriptors, sandbox path validation |
| `pluribusfabric` | Control-frame header/encoding, bounded incremental decoder, provisional bootstrap message types |
| `pluribusscheduler` | Pure V1 scheduler policy with exact integer/rational scoring |
| tests | Unit tests for the core model, fabric framing and the documented scheduler cases |

Not implemented yet: TCP transport and session handshake, `pluribusmaster`,
`pluribusnode`, `pluribussim` backends, artifact store, public C++ client, and
the `plu` CLI. The wire serialization format and runtime ABI remain open design
decisions.

## Build and test

Requirements: CMake 3.28+, Ninja, and a C++23 compiler. The initial presets
use GCC on Windows, GCC on Linux, and AppleClang/Clang on macOS.

```sh
cmake --preset windows-gcc-debug
cmake --build --preset windows-gcc-debug
ctest --preset windows-gcc-debug
```

On Linux use `linux-gcc-debug`; on macOS use `macos-appleclang-debug`.
Warnings can be promoted to errors with `-DPLURIBUS_WARNINGS_AS_ERRORS=ON`.

The test harness is a deliberately dependency-free placeholder so the first
targets build without a package manager. Replace or wrap it when the vcpkg
manifest and dependency baseline are introduced.

## Repository layout

```text
core/               platform-independent model and contracts
fabric/             control framing and transport
master/scheduler/   pure scheduling policy
tests/              unit tests and test support
tools/              repository checks and developer scripts
cmake/              compiler options and shared CMake helpers
docs/               architecture, specifications, second-brain layer and task reports
```
