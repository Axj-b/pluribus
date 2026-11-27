---
type: note
id: cli
status: active
maturity: growing
tags: [note, cli, api, ux]
aliases: ["plu", "CLI"]
sources: ["../architecture/08-roadmap.md#cli-plu", "../spec/03-cpp-public-api.md"]
last_reviewed: 2026-11-25
---

# CLI (`plu`)

> `plu` is the architecture test: it uses exactly the public protocol/API that any application uses.
> It must not have a privileged back door.

## Commands

| Command | Purpose |
|---|---|
| `plu nodes` | Registered nodes and resource state |
| `plu run <exe> [args]` | Run finite execution, wait, stream stdout/stderr, return remote exit code |
| `plu run --detach <exe> [args]` | Return immediately with `ExecutionId` |
| `plu ps` | List executions |
| `plu logs <id>` | Stream logs |
| `plu stop <id>` | Terminate execution |
| `plu services` | List services |
| `plu resolve <service>` | Test discovery |

## Behavior

- `--json` for scripts.
- Configuration precedence: CLI > environment > user config > system/default config.
- Foreground interrupt may request cancellation; it is not proof of termination.
- `--detach` stops waiting; it does not change the execution's lifetime class.
- Executable source follows the explicit syntax in the package model; no node `PATH` lookup.

## Why it matters

The CLI exercises connect, submit, handle, state observation, log streaming, termination and service
resolution end to end. If the CLI needs an internal shortcut, the public API is wrong.

## Related

- [End-user guide](../guides/end-user-guide.md)
- [Execution](execution.md)
- [Services](service.md)
- [Packages](package.md)
- [Public API](../spec/03-cpp-public-api.md)

## Canonical sources

- [CLI commands](../architecture/08-roadmap.md#cli-plu)
- [Public C++ API](../spec/03-cpp-public-api.md)
- [Executable source syntax](../spec/08-artifact-store-protocol.md#submission-inputs-and-outputs)

## Open questions

- [OI-10 Public API ownership and async model](../questions/README.md#oi-10-public-api-ownership-and-async-model)
- [OI-28 Minimal hello resolution](../questions/README.md#oi-28-minimal-hello-resolution)
