# 06. Node/runtime boundary

Status: V1 design specification; C++ interfaces and configuration below are illustrative, not a
published API. See [execution model](01-execution-model.md), [node/master protocol](02-node-master-protocol.md),
[service model](04-service-model.md), [wire protocol](05-wire-protocol.md), and
[security model](10-security-model.md).

## Responsibilities

`pluribusnode` is the per-host agent. It registers and heartbeats, receives assignments and
commands, owns execution records and cache pins, fetches inputs, coordinates materialization and
output publication, reports state and measurements, and forwards logs and service control. It
has no knowledge of the execution's application domain. `pluribussim` implements the uniform
execution environment: sandbox setup, path translation for runtime operations, clean environment,
process launch/supervision, streams, local service endpoint, and host measurements through a
backend. The agent drives the runtime; neither client disconnection nor control connection loss
alone stops a process.

Pluribus is **not a Linux simulator** and V1 does not implement Linux syscall emulation, a virtual
filesystem, CPU emulation, or a general network stack. Applications may use ordinary networking
and hardware independently of Pluribus services.

## Host backend

The following sketch conveys ownership and error boundaries, not exact signatures or ABI:

```cpp
#include <cstddef>
#include <expected>
#include <span>

namespace pluribus::runtime {
struct ProcessSpec; struct ProcessHandle; struct ProcessStatus;
struct Path; struct Pipe; struct LocalEndpoint; struct HostResources;
struct ProcessUsage; struct Error;
template<class T> using Result = std::expected<T, Error>;

class HostBackend {
public:
    virtual ~HostBackend() = default;
    virtual Result<ProcessHandle> spawn(const ProcessSpec&) = 0;
    virtual Result<void> terminate(const ProcessHandle&) = 0;
    virtual Result<void> kill(const ProcessHandle&) = 0;
    virtual Result<ProcessStatus> query(const ProcessHandle&) = 0;
    virtual Result<ProcessUsage> measure(const ProcessHandle&) = 0;
    virtual Result<void> create_directory(const Path&) = 0;
    virtual Result<std::size_t> read_file(const Path&, std::span<std::byte>) = 0;
    virtual Result<void> write_file(const Path&, std::span<const std::byte>) = 0;
    // File and pipe operations use opaque, runtime-owned handles in the real interface.
    virtual Result<Pipe> create_pipe() = 0;
    virtual Result<LocalEndpoint> create_local_endpoint() = 0;
    virtual Result<HostResources> host_resources() = 0;
};
}
```

Real definitions also need bounded file read/write, close and cleanup operations, stream ownership,
and asynchronous integration. Backend-only objects encapsulate host handles and native errors;
public types expose Pluribus errors, not PIDs or OS handles. `win32`, `linux`, `macos`, and
`driveos` backends implement the same contract; DRIVE OS Linux is the initial DRIVE target.
Select the implementation at build/composition time. Platform headers stay in backend code.

## Process lifecycle and measurement

1. Agent validates the assignment, pins and materializes inputs, creates directories, and asks the
   runtime to spawn the selected entry point with explicit arguments, working directory, and
   environment. A failed stage reports a typed error and releases its resources.
2. The runtime captures stdout and stderr separately, timestamps chunks, and reports running and
   exit observations. Stdin is closed unless explicitly supplied. Backend launch must not invoke a
   shell implicitly. Normal process exit is reported with its exit status.
3. `terminate` requests graceful stop of the execution **process tree**; `kill` requests forced
   stop of that tree. Map these logical operations to each host's mechanisms. A grace deadline and
   any escalation are explicit master policy, not an implied guarantee of `terminate` itself.
   Repeated commands are safe; report failure or incomplete tree handling rather than pretending
   success. V1 process-group/tree supervision is operational best effort, not containment against
   a hostile process that escapes. Stronger Job Objects and similar enforcement are future work.
4. After process exit, the agent collects declared outputs, reports the outcome and releases pins
   only when collection/transfer no longer needs them. State and races are governed by the
   [execution model](01-execution-model.md).

Measure CPU time and memory/working set per execution where the backend can do so reliably;
report unavailable measurements as unknown, never as zero. Node CPU, memory, architecture and
discoverable accelerators are reported separately. Measurements inform accounting and later
profiles, not V1 hard resource limits. See [scheduler](07-scheduler-v1.md).

Stdout/stderr are ordinary byte streams, not a required application logging API. Keep bounded,
timestamped node history (initially about 10 MiB per execution, master-configurable), then replay
retained history before live subscription. Slow/disconnected readers must not block supervision;
bounded storage may discard oldest records, and a subscriber must be told of any replay gap.
Structured component logs include timestamp, component, node, execution, severity and event.

## Sandbox and paths

Each execution has a conceptual root `/pluribus/executions/<ExecutionId>/` with `input/`, `work/`,
`output/`, and `tmp/`; this is a logical description, **not** a mounted path in V1. The node
maps it to an internal configured physical sandbox root. Work normally runs with `work/` as its
current directory, so an application uses relative paths. Input/output specifications identify
validated paths inside the sandbox; see [store](08-artifact-store-protocol.md).

V1 may pass `PLURIBUS_INPUT`, `PLURIBUS_WORK`, `PLURIBUS_OUTPUT`, `PLURIBUS_TMP`, and a local
service endpoint in `PLURIBUS_SERVICE_ENDPOINT` as native path/address values where needed. These
are an explicit transitional bridge: their values can reveal physical host details to the process
and must not become persistent logical identifiers or public API paths. There is no illusion that
a Windows process can open a conceptual Unix path. Prevent traversal, unsafe links and extraction
outside the root in runtime-managed operations, but do not claim a malicious native process is
confined. See [security](10-security-model.md).

The master configures retention: successful temporary executions have short retention, failures
longer retention for diagnosis, and persistent executions retain their sandbox while active.
Actual durations are operator policy, not wire constants. Cleanup must not delete active data or
outputs still being published; persist required results as artifacts, not permanent sandboxes.

## ABI, identity and configuration

Scheduling compatibility is **Pluribus ABI version plus CPU architecture**, e.g.
`pluribus-v1-x86_64`, not host OS as a scheduling capability. Version the runtime contract and
reject incompatible versions; do not assume one architecture runs another. Native Windows and
Linux binaries are not interchangeable merely because their CPU matches. The actual portable
native ABI/runtime mechanism is not yet demonstrated; the key is a target contract, not evidence
that arbitrary native binaries work everywhere. See [package design](08-artifact-store-protocol.md).

Persist a generated UUID as `NodeId` across agent restarts. A separately configurable, changeable
human-readable name is display/CLI metadata, not identity; duplicated names must not silently
identify a node. Config example, illustrative rather than a fixed schema:

```toml
[node]
name = "thor-dev"
identity_file = "state/node-id"
capabilities = ["target.thor", "device.radar.front"]

[master]
address = "192.0.2.10:7400"
join_token_file = "secrets/node-token"

[runtime]
sandbox_root = "state/executions"

[cache]
root = "state/cache"
capacity_bytes = 21474836480
```

Architecture, CPU, memory, ABI and reliably detectable accelerators are discovered; configured
capabilities supplement them. Resolve relative config paths against the config file's directory.
Master policies control retention and reservations; node config controls local paths and cache
capacity. The node runs in foreground in V1, with no automatic self-update.

## Disconnect, reconciliation and services

On master loss the node keeps supervising existing processes, buffers events within configured
bounds, does not accept new globally scheduled work, and reconnects. It reports its identity and
active execution IDs, plus retained state/events needed for reconciliation. The master adopts a
known still-running execution only when identity is established; persistent and finite executions
are both eligible. It applies configured orphan policy to unknown/stale executions rather than
blindly killing them. A `Lost` observation is not proof of process death. In-memory master state
may be lost on restart; do not claim all orphaned executions can be adopted without evidence.
Details, sequence handling and recovery of terminal events belong to the
[node/master protocol](02-node-master-protocol.md) and [wire protocol](05-wire-protocol.md).

For services, expose a local language-neutral endpoint as well as the preferred C++ client API.
It uses documented, versioned protocol messages, binds operations to the execution identity, and
supports service registration/readiness and logical connection. Endpoint implementation is host
internal; applications must not depend on a particular socket path or physical address. Service
publication, readiness and byte-stream semantics are specified in [04](04-service-model.md).

## Open issues

- Prove and specify the native Pluribus ABI across supported OSes; binary format, system libraries
  and process-tree guarantees require platform prototypes.
- Fix concrete backend/local endpoint signatures, logging gap representation, retention defaults,
  and the supported environment variables when wire/API implementations exist.
