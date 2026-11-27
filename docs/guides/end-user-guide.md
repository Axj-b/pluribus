---
type: guide
id: end-user-guide
status: draft
tags: [guide, cli, ux, user]
aliases: ["User guide", "How to use Pluribus"]
last_reviewed: 2026-11-27
---

# Pluribus end-user guide

> **Status: planned interface — not yet runnable.** This describes the target user experience from the
> [roadmap](../architecture/08-roadmap.md#cli-plu) and the [execution model](../spec/01-execution-model.md).
> Today only the core model, scheduler and fabric layers are implemented; the `plu` CLI, the master and
> the node agent do not exist yet. Everything below shows the intended commands and behavior, not a
> working install.

## What Pluribus is

Pluribus runs a job on *some machine you don't have to name*, and streams the output back to you. You
submit an **execution**; the **master** picks a suitable **node**, the node runs it in an isolated
workspace, and you get the standard output/error and the exit code.

The pieces you'll see as a user:

| Term | Meaning |
|---|---|
| **master** | The coordinator: registers nodes, queues and schedules work. |
| **node** | A machine that contributes CPU, RAM and capabilities and runs executions. |
| **execution** | One submitted job (finite or persistent), with a stable ID and state. |
| **artifact** | Immutable, content-addressed input or output (a `sha256:` digest). |
| **service** | A named, long-lived provider a job can connect to (later milestone). |

## The quick tour

```sh
plu nodes                     # which nodes exist and their resource state
plu run hello                 # run "hello", stream output, wait, return its exit code
plu run --detach hello        # same, but return immediately with an execution ID
plu ps                        # list your executions and their states
plu logs <id>                 # stream stdout/stderr of a running or finished execution
plu stop <id>                 # terminate an execution
```

`plu run hello` is the whole point of the first milestone: it prints `hello`'s output on your
machine while the process actually ran on a remote node, and it exits with the same code the remote
process produced.

## What you can put after `run`

The executable argument uses explicit syntax — there is **no** `PATH` lookup on the node:

| Syntax | Meaning |
|---|---|
| `hello` | Package name (resolved once to immutable content). |
| `hello@1.4.2` | An exact package version. |
| `artifact:sha256:<digest>` | Immutable content by its hash. |
| `./hello` | A local file, uploaded by the client before the run. |

## Execution lifecycle

An execution moves through ordered states, and `plu ps` reports them:

```
created → queued → assigned → materializing → starting → running → completed | failed | terminated | lost
```

`completed` means the process exited zero; `failed` means non-zero or abnormal exit. `lost` is a
provisional "we can't see it" state that can be corrected by reconciliation — it does not silently
mean the work stopped.

## Scripting

- `--json` makes every command emit machine-readable JSON.
- `plu run`'s exit code is the **remote** exit code, so shell pipelines work as expected.
- Foreground interrupt requests cancellation; it is not proof the remote process stopped — use
  `plu stop` to terminate deliberately.
- `--detach` only stops waiting; it does not change the execution's lifetime class.

## Configuration

Precedence is, from highest to lowest:

```
CLI flags  >  environment  >  user config  >  system/default config
```

In the first milestone the master address is static configuration (there is no service discovery
yet), and the network is assumed trusted — see the [security model](../spec/10-security-model.md).

## What exists today vs. what is planned

| Area | Status |
|---|---|
| Core model, IDs, state machine | implemented |
| Scheduler (pure V1 policy) | implemented |
| Wire framing / transport / handshake / control codec | implemented |
| `plu` CLI, master, node agent, runtime backends | **not implemented** |
| Artifact store, services | **not implemented** |

When the first end-to-end slice lands, this guide will be updated to reflect the real commands and a
working `hello` example.

## Related

- [CLI note](../notes/cli.md)
- [Roadmap](../architecture/08-roadmap.md)
- [Execution model](../spec/01-execution-model.md)
