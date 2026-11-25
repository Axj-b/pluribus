---
type: note
id: sandbox
status: active
maturity: growing
tags: [note, sandbox, paths, runtime]
aliases: ["Execution sandbox", "Logical paths"]
sources: ["../spec/06-node-runtime-boundary.md", "../architecture/04-node-and-runtime.md", "../spec/10-security-model.md"]
last_reviewed: 2026-11-25
---

# Sandbox

> Every execution gets an isolated logical workspace. In V1 this is an **operational** workspace,
> not a security boundary.

## Logical form

```text
/pluribus/executions/<ExecutionId>/
  input/
  work/
  output/
  tmp/
```

The physical location is node-specific and never a public identity. Work normally starts in `work/`
so applications use relative paths.

## Rules

- Inputs and outputs are sandbox-relative validated paths.
- Reject absolute paths, drive/UNC prefixes, empty or dot/dot-dot components, backslashes, embedded
  NUL, duplicate destinations and unsafe aliases.
- Resolved destinations must stay under the intended subtree, including through links/reparse
  points. Use handle-relative/no-follow operations where available and fail closed if confinement
  cannot be established.
- V1 does not promise path virtualization. A native process sees native paths through the
  `PLURIBUS_*` bridge.
- Retention is master-configured: short for successful temporary runs, longer for failures,
  while active for persistent executions. Persistent results belong in artifacts, not permanent
  sandboxes.
- Cleanup must not delete active data or outputs still being published.

## Security note

A native execution runs with the node's OS privileges. The sandbox does not confine a hostile
process. See [Security](security.md) before claiming isolation.

## Related

- [Runtime](runtime.md)
- [Node](node.md)
- [Execution](execution.md)
- [Artifact store](artifact-store.md)
- [Security](security.md)

## Canonical sources

- [Runtime boundary: sandbox and paths](../spec/06-node-runtime-boundary.md#sandbox-and-paths)
- [Architecture: execution sandbox](../architecture/04-node-and-runtime.md#44-execution-sandbox)
- [Security: paths, environment and artifacts](../spec/10-security-model.md#paths-environment-and-artifacts)

## Open questions

- [OI-17 Host backend signatures and environment](../questions/README.md#oi-17-host-backend-signatures-and-environment)
- [OI-24 Hard isolation threat model](../questions/README.md#oi-24-hard-isolation-threat-model)
