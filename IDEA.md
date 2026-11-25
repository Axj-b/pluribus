# Pluribus Architecture

## 1. Purpose

Pluribus is a portable distributed execution environment that combines heterogeneous computers into one logical machine.

A Pluribus application should not care whether it physically executes on:

- Windows
- Linux
- macOS
- NVIDIA DRIVE AGX Thor
- another future embedded platform

The physical operating system is an implementation detail below the Pluribus runtime.

The fundamental abstraction is:

**Execution**

An execution can be anything:

- `hello`
- a compiler invocation
- a unit test
- a CMake process
- RadarScape
- a RadarScape processing worker
- a long-running service
- an AI worker
- a Thor integration test

Pluribus does not assign semantic meaning to those activities.

---

# 2. System model

Conceptually:

**Pluribus Machine**

`Pluribus Application → Pluribus Environment → Pluribus Fabric → Pluribus Node → Host`

A deployment might contain:

**Master workstation**

- `pluribusmaster`
- `pluribussim`
- 16 x86-64 cores
- 64 GB RAM
- NVIDIA GPU
- Windows

**Linux worker**

- `pluribussim`
- 32 x86-64 cores
- 128 GB RAM
- Linux

**Thor worker**

- `pluribussim`
- ARM64
- 64 GB RAM
- NVIDIA GPU
- DRIVE OS

Applications above `pluribussim` should normally see only:

- Pluribus architecture
- Pluribus filesystem
- Pluribus processes
- Pluribus services
- Pluribus resources
- Pluribus packages

They should not see the host operating system.

---

# 3. Architectural principles

## 3.1 Execution is the primitive

Do not introduce concepts such as:

- BuildJob
- CompileJob
- TestJob
- RadarJob
- AIJob

The core API understands only an `Execution`.

Conceptually:

**ExecutionSpec**

- executable
- arguments
- environment
- working directory
- input artifacts
- expected outputs
- lifetime
- optional constraints
- optional exported services

Everything else belongs above Pluribus.

CMake can create compiler executions.

RadarScape can create worker executions.

An AI system can create inference executions.

Pluribus treats them identically.

---

## 3.2 The scheduler has no domain knowledge

The scheduler must never need to understand what an executable does.

It answers only:

> Which compatible Pluribus node should execute this request?

It can consider:

- CPU architecture
- Pluribus ABI version
- available CPU
- available RAM
- available accelerator capabilities
- current load
- cached artifacts
- required devices
- explicit placement constraints
- previous runtime information

It must not contain logic concerning:

- C++
- CMake
- RadarScape
- SAR
- unit tests
- AI
- NVIDIA
- automotive development

---

## 3.3 Host operating systems stay below pluribussim

Applications see:

`Application → Pluribus API/ABI → pluribussim → host backend`

Possible host implementations:

- `backend_win32`
- `backend_linux`
- `backend_macos`
- `backend_driveos`

Do not expose host-specific handles, paths, errors, process identifiers, sockets, or filesystem semantics through the public Pluribus API unless there is no viable alternative.

---

## 3.4 Architecture remains explicit

Host operating systems can be abstracted.

CPU instruction sets cannot be hidden without translation.

Initial architectures:

- `x86_64`
- `arm64`

A Pluribus package can therefore contain multiple native implementations.

Example:

**`radar-worker.plp`**

- manifest
- `bin/x86_64/radar-worker`
- `bin/arm64/radar-worker`
- resources

The scheduler selects a node for which the package contains a compatible executable.

Pluribus does not emulate ARM64 on x86-64 or vice versa in the initial architecture.

---

## 3.5 Distribution happens at execution boundaries

Pluribus distributes:

- executions
- files/artifacts
- streams
- services

Pluribus does not initially implement distributed shared memory.

This distinction is important.

Ethernet is treated as a transport for coarse-grained work rather than as a memory bus.

---

# 4. Primary components

The initial system should contain six major components.

## `pluribuscore`

Shared types and protocols.

## `pluribusmaster`

Global orchestration and scheduling.

## `pluribusnode`

Agent running on every physical machine.

## `pluribussim`

Uniform execution/runtime environment.

## `pluribusfabric`

Communication between Pluribus components.

## `pluribusstore`

Content-addressed artifact storage and caching.

Package management and distributed filesystem functionality should come later.

---

# 5. pluribuscore

`pluribuscore` contains platform-independent definitions.

It should have nearly zero runtime policy.

Responsibilities:

- identifiers
- execution definitions
- execution state
- node descriptions
- resource descriptions
- service descriptions
- artifact identifiers
- protocol messages
- errors
- serialization contracts

Example strong types:

- `NodeId`
- `ExecutionId`
- `ArtifactId`
- `ServiceId`
- `PackageId`

Avoid passing arbitrary strings for these identifiers internally.

---

# 6. Execution model

## 6.1 ExecutionSpec

The central frontend object.

Conceptually it contains:

**Identity**

- executable reference
- package reference if applicable

**Arguments**

- executable arguments
- environment
- working directory

**Inputs**

- artifact references
- mounted directories
- read-only/read-write policy

**Outputs**

- declared artifact paths where relevant

**Lifecycle**

- finite
- persistent

**Services**

- exported service names

**Constraints**

Optional:

- architecture
- required capabilities
- preferred capabilities
- minimum memory
- minimum CPU
- device requirements
- explicit node

Constraints should be optional because the runtime should derive as much compatibility information as possible.

---

## 6.2 ExecutionHandle

Starting an execution returns a handle rather than blocking.

Conceptually:

**ExecutionHandle**

- execution ID
- state
- assigned node
- service endpoints
- stdout stream
- stderr stream

Operations:

- wait
- terminate
- kill
- query state
- subscribe logs
- query service
- fetch outputs

The same interface works for both short-lived and persistent processes.

---

## 6.3 Execution states

Use an explicit state machine.

Suggested states:

`Created → Queued → Assigned → Materializing → Starting → Running`

From `Running`:

- `Completed`
- `Failed`
- `Terminated`
- `Lost`

Do not infer execution state from transport connection state.

---

# 7. pluribusmaster

`pluribusmaster` represents the control plane of one Pluribus machine.

Responsibilities:

- node registry
- node health
- execution queue
- scheduling
- execution ownership
- placement decisions
- service registry
- global namespace
- distributed state
- artifact location information

It does **not** execute applications itself unless its machine also runs an `pluribusnode`.

A master workstation will normally run both.

---

# 8. Scheduler

The scheduler consumes:

- an `ExecutionSpec`
- current node state
- compatibility information
- optional historical resource profile

Its output is:

- `NodeId`

Nothing more.

Scheduling pipeline:

1. Determine execution compatibility.
2. Remove unavailable nodes.
3. Remove architecture-incompatible nodes.
4. Remove nodes missing mandatory capabilities.
5. Evaluate current resource availability.
6. Consider data/cache locality.
7. Score remaining candidates.
8. Select node.
9. Reserve resources.
10. Dispatch execution.

Version 1 should use an intentionally simple score.

For example:

- compatibility is mandatory
- offline node: reject
- insufficient declared memory: reject
- otherwise prefer lowest normalized load
- use local node as tie breaker

Do not build predictive scheduling first.

---

# 9. Node capability model

Every `pluribusnode` periodically reports a `NodeDescriptor`.

Static information:

- node ID
- Pluribus protocol version
- Pluribus ABI version
- architecture
- total logical CPU count
- total memory
- available accelerator classes
- persistent device capabilities

Dynamic information:

- current CPU utilization
- available memory
- running execution count
- cache state summary
- accelerator utilization
- health

Capabilities should support extensible names.

Examples:

- `gpu.cuda`
- `gpu.vulkan`
- `target.thor`
- `device.radar.front`
- `vehicle.can`
- `sensor.camera.360`

CPU and RAM should remain quantitative resources rather than capability tags.

---

# 10. pluribusnode

`pluribusnode` is deliberately simple.

Responsibilities:

- connect/register with master
- report capabilities
- heartbeat
- receive execution assignments
- materialize inputs
- create sandbox
- invoke pluribussim
- monitor process
- collect resource statistics
- expose services
- stream stdout/stderr
- terminate executions
- collect outputs
- maintain artifact cache

`pluribusnode` has no understanding of why an execution exists.

---

# 11. pluribussim

`pluribussim` provides the common environment that removes host-OS differences.

Initial responsibilities:

- process launch
- consistent environment structure
- logical paths
- environment variables
- execution sandbox
- standard streams
- process lifecycle
- service registration integration
- resource measurements
- host translation

The first version should **not** attempt to implement a complete Linux syscall ABI.

Instead, start by standardizing the execution environment around binaries built for the available native architectures and runtime contract.

The Linux-like ABI/runtime question can be developed behind this boundary later.

---

# 12. pluribussim backend boundary

Define a small host interface.

Conceptually:

**HostBackend**

- spawn process
- stop process
- kill process
- query process
- create directory
- read/write file
- create pipe
- create local endpoint
- query CPU
- query memory
- query architecture
- query accelerator capabilities

Implementations:

- `Win32HostBackend`
- `LinuxHostBackend`
- `MacHostBackend`
- eventually `DriveOSHostBackend`

Most of Pluribus should compile without including Windows, Linux, or macOS platform headers.

---

# 13. Execution sandbox

Every execution receives an isolated working area.

Logical form:

`/pluribus/executions/<ExecutionId>/`

Containing roughly:

- `input/`
- `work/`
- `output/`
- `tmp/`

The physical location is node-specific.

Windows might use:

`C:\ProgramData\Pluribus\executions\...`

Linux might use:

`/var/lib/pluribus/executions/...`

The execution itself should see only Pluribus logical paths.

Version 1 isolation can be operational rather than security-grade.

Do not claim that a Pluribus sandbox is a security boundary until actual OS isolation mechanisms are introduced and audited.

---

# 14. pluribusfabric

`pluribusfabric` is the communication substrate.

It should support two distinct planes.

## Control plane

Small messages:

- registration
- heartbeat
- execution request
- execution state
- resource updates
- service registration
- terminate
- errors

## Data plane

Potentially large streams:

- artifacts
- application streams
- RadarScape data
- logs
- recordings

Keep these conceptually separate even if they initially share one TCP implementation.

A 2 GB radar recording must not block a process termination message.

---

# 15. Initial network transport

Do not invent a raw Ethernet protocol.

Version 1:

- TCP
- one control connection per node
- separate data connections
- length-prefixed binary messages
- explicit protocol version

Later candidates:

- QUIC
- TLS
- compression
- RDMA
- shared-memory transport
- zero-copy networking

Only introduce them when profiling demonstrates a requirement.

---

# 16. Service model

Persistent executions need location-transparent communication.

An application exports a logical service.

Example:

`radar-slave/control`

Master registry records:

`ServiceId → ExecutionId → NodeId → endpoint`

Consumers request:

`pluribus://radar-slave/control`

They do not receive or store physical topology permanently.

Routing decides how the connection is implemented.

Same-node:

`client → local IPC/shared memory → service`

Cross-node:

`client → Pluribus fabric → Ethernet → service`

The application's logical addressing remains identical.

---

# 17. ServiceRegistry

`pluribusmaster` initially owns the authoritative service registry.

Operations:

- register
- unregister
- resolve
- watch
- health/query

Service registrations are associated with execution lifetime.

When an execution terminates, its services disappear automatically.

Do not make applications manually clean up stale service entries.

---

# 18. pluribusstore

Files should eventually be transferred as content-addressed artifacts.

An artifact identity should primarily derive from its contents.

Example:

`ArtifactId = SHA-256(content)`

Instead of:

> Send `foo.hpp` to node 3.

Master can say:

> Execution needs artifact `A91F...`.

Node checks cache.

If present:

- reuse

If absent:

- fetch

This enables automatic caching across executions.

---

# 19. Artifact model

Core object:

**Artifact**

- immutable hash
- size
- optional logical name
- optional metadata

Directories can later be represented as manifests containing artifact references.

Benefits:

- deduplication
- caching
- deterministic inputs
- reproducibility
- incremental synchronization

Do not attempt a fully distributed POSIX filesystem in version 1.

---

# 20. Future pluribusfs

`pluribusfs` can eventually present a unified namespace:

- `/work`
- `/packages`
- `/artifacts`
- `/tmp`

But initially `/work` should simply be materialized into the execution sandbox.

A distributed filesystem introduces:

- cache coherence
- locking
- consistency semantics
- metadata synchronization
- failure recovery

Those problems are not required to prove Pluribus.

---

# 21. Packaging

Later introduce `.plp` packages.

Conceptual structure:

**package manifest**

- package name
- version
- Pluribus ABI version
- supported architectures

**native executables**

- x86-64
- ARM64

**resources**

- configuration
- models
- shader files
- other static data

Example package:

`radarscape-sar-worker.plp`

contains:

- x86_64 implementation
- ARM64 implementation
- resources

Node compatibility is determined automatically from package metadata.

---

# 22. Host-independent networking

Applications should preferably use Pluribus service APIs rather than directly relying on physical network addresses.

Bad:

`connect("192.168.10.44", 5800)`

Preferred:

`connect("sar-worker/data")`

This permits Pluribus to optimize transport depending on placement.

Future same-node communication can become shared memory without changing the application protocol.

---

# 23. RadarScape example

RadarScape master requests three persistent executions:

- DetectionWorker
- SARWorker
- VisualizationWorker

Scheduler chooses:

- DetectionWorker → workstation node
- SARWorker → Linux mini-PC
- VisualizationWorker → GPU node

Applications publish:

- `detection/output`
- `sar/output`
- `visualization/control`

RadarScape resolves those logical services.

Physical IP addresses never become RadarScape configuration.

If SARWorker later moves to another node, service resolution changes without changing RadarScape's logical topology.

---

# 24. Failure model

Distributed systems fail differently from local processes.

Pluribus must distinguish:

**Process failure**

Application exited unexpectedly.

**Node failure**

Node stopped responding.

**Fabric failure**

Node may still be alive but unreachable.

**Master failure**

Control plane unavailable.

Initial behavior should be conservative.

For finite executions:

- mark `Lost`
- optionally allow caller-requested retry later

For persistent executions:

- mark `Lost`
- notify owner
- do not automatically restart elsewhere initially

Automatic recovery and migration come after lifecycle semantics are stable.

---

# 25. Master ownership

Version 1 should use one authoritative master.

Do not start with master clustering or consensus.

One Pluribus machine:

- one `pluribusmaster`
- N `pluribusnode`s

If master dies:

- existing processes can continue locally
- no new scheduling occurs
- master state can initially be considered lost/reconstructible

High availability can be addressed later.

---

# 26. Repository boundaries

Recommended initial repository:

**`Pluribus/`**

- `CMakeLists.txt`
- `cmake/`
- `core/`
  - types
  - execution
  - node
  - resources
  - services
  - artifacts
  - protocol
- `master/`
  - registry
  - scheduler
  - execution_manager
  - service_registry
- `node/`
  - agent
  - executor
  - monitor
  - cache
- `runtime/`
  - pluribussim
  - sandbox
  - environment
- `platform/`
  - win32
  - linux
  - macos
- `fabric/`
  - protocol
  - transport
  - connection
  - streams
- `store/`
  - artifact
  - hash
  - local_cache
- `cli/`
  - plu
- `tests/`
  - unit
  - integration
  - system

Do not split these into separate repositories initially.

The boundaries need to stabilize first.

---

# 27. Dependency direction

Keep dependencies strict.

`pluribuscore`

depends on nothing Pluribus-specific.

`pluribusfabric → pluribuscore`

`pluribusstore → pluribuscore`

`pluribussim → pluribuscore`

`pluribusnode → pluribuscore + pluribusfabric + pluribusstore + pluribussim`

`pluribusmaster → pluribuscore + pluribusfabric + pluribusstore`

`plucli → pluribuscore + pluribusfabric`

Platform backends are dependencies of `pluribussim`, never the other way around.

Forbidden dependency:

`pluribuscore → Windows/Linux/macOS`

Also forbidden:

`pluribusmaster → RadarScape`

`pluribusmaster → CMake`

`pluribusmaster → Thor`

Application domains stay outside the platform.

---

# 28. Public API boundary

Keep the initial public API extremely small.

Required concepts:

- connect to Pluribus
- submit execution
- obtain execution handle
- observe state
- stream stdout/stderr
- terminate execution
- resolve service

Everything else should remain internal until required.

Avoid designing a large distributed-computing framework API before real applications exercise it.

---

# 29. CLI

Build a simple `plu` client early because it provides an excellent architecture test.

Initial commands:

`plu nodes`

Displays registered nodes and resource state.

`plu run hello`

Creates finite execution.

`plu ps`

Shows executions.

`plu logs <id>`

Streams logs.

`plu stop <id>`

Terminates execution.

`plu services`

Lists registered services.

`plu resolve radar-slave/control`

Tests service discovery.

This CLI should use exactly the same public protocol/API that RadarScape or another application would use.

---

# 30. Prototype 1: remote hello

Two computers.

**Machine A**

- pluribusmaster
- pluribusnode
- Pluribus CLI

**Machine B**

- pluribusnode

Run:

`plu run hello`

System flow:

1. CLI submits ExecutionSpec.
2. Master queues it.
3. Scheduler chooses node B.
4. Node B receives assignment.
5. Inputs are materialized.
6. pluribussim creates sandbox.
7. `hello` starts.
8. stdout streams to master/client.
9. `hello` exits.
10. exit status propagates.
11. execution becomes Completed.

Success criterion:

`Hello world`

appears on Machine A even though execution physically happened on Machine B.

---

# 31. Prototype 2: persistent service

Run:

`plu run --persistent radar-slave`

Slave registers:

`radar-slave/control`

Master returns an ExecutionHandle and service identity.

A client then:

1. resolves `radar-slave/control`
2. opens connection
3. sends message
4. receives response
5. closes

Success criterion:

The client never knows the worker's physical IP address.

---

# 32. Prototype 3: heterogeneous nodes

Add:

- one Windows node
- one Linux node

Both execute the same Pluribus-compatible application package.

Success criterion:

`plu run hello`

can be scheduled to either host without the caller changing the command.

This is the first real proof that Pluribus successfully hides the host OS.

---

# 33. Prototype 4: architecture packaging

Add ARM64 package variant.

Package contains:

- x86_64 hello
- ARM64 hello

Add Thor or another ARM64 node.

Scheduler automatically chooses the correct binary variant based on node architecture.

Success criterion:

The logical command remains:

`plu run hello`

regardless of x86-64 or ARM64 placement.

---

# 34. Prototype 5: RadarScape worker

Only after the generic infrastructure works:

1. Package a simple RadarScape processing service.
2. Start through Pluribus.
3. Register a data service.
4. Connect from RadarScape.
5. Send synthetic radar data.
6. Process remotely.
7. Return results.

Do not start with full ADC streaming.

Begin with small synthetic messages and establish correct lifecycle/service semantics first.

---

# 35. What not to build yet

Explicitly defer:

- Linux syscall emulation
- modified musl
- distributed POSIX filesystem
- master clustering
- scheduling AI
- predictive load models
- process migration
- transparent distributed RAM
- containers
- Kubernetes integration
- sophisticated security sandboxing
- GPU workload migration
- ARM emulation
- raw Ethernet protocol
- automatic compiler integration

None of these are needed to prove the platform.

---

# 36. First implementation order

## Milestone A: Core model

Implement:

- IDs
- ExecutionSpec
- ExecutionHandle/state
- NodeDescriptor
- resources
- ServiceDescriptor
- protocol definitions

## Milestone B: Fabric

Implement:

- master listener
- node connection
- framing
- serialization
- heartbeat

## Milestone C: Node registry

Implement:

- node registration
- CPU/RAM reporting
- health timeout
- `plu nodes`

## Milestone D: Remote execution

Implement:

- queue
- trivial scheduler
- assignment
- process launch
- stdout/stderr forwarding
- exit status
- `plu run`

## Milestone E: Lifecycle

Implement:

- `plu ps`
- `plu stop`
- persistent executions
- node-loss detection

## Milestone F: Services

Implement:

- register
- resolve
- proxy/forward stream
- service lifetime binding

## Milestone G: Artifacts

Implement:

- SHA-256 IDs
- local cache
- transfer
- execution input materialization
- outputs

Only after G should a serious RadarScape worker be introduced.

---

# 37. Technology choices for V1

Language:

**C++23**

Build:

**CMake + Ninja**

Serialization:

Use a proven schema system rather than inventing binary object serialization. Protocol Buffers is a reasonable initial choice.

Hashing:

**SHA-256**

Transport:

**TCP**

Concurrency:

Use asynchronous I/O where networking requires it, but avoid building a complex actor framework initially.

Testing:

- GoogleTest for unit tests if already acceptable in the project
- process-level integration tests
- two-node loopback system tests
- eventually real multi-host CI

Logging:

Use structured logs containing at least:

- timestamp
- component
- node ID
- execution ID
- severity
- event

---

# 38. Security model for V1

Assume trusted development machines on a trusted network.

Still implement:

- protocol version validation
- message-size limits
- path traversal protection
- execution directory confinement
- clean environment construction
- explicit artifact hashes
- node identity

Do not market V1 as safe for executing untrusted code.

Later:

- TLS
- node certificates
- signed packages
- Windows Job Objects
- Linux namespaces/cgroups/seccomp
- macOS sandbox mechanisms
- privilege separation

can harden the architecture without changing the execution abstraction.

---

# 39. Performance targets for the prototype

Use explicit targets so architectural regressions are visible.

Local master request overhead:

**< 1 ms target**

LAN scheduling + control dispatch:

**< 10 ms target on a normal 1 Gbit Ethernet development LAN**

Small persistent-service round trip:

**< 2 ms additional Pluribus overhead target on a local wired LAN**

Node heartbeat:

**1 second**

Node considered unavailable:

**3–5 missed heartbeats**

Do not optimize artifact throughput until actual build/RadarScape workloads are measured.

---

# 40. The architectural invariant

The most important invariant is:

> **A Pluribus application identifies what it wants to execute and communicate with, never where that work physically lives.**

Therefore:

Application decides:

**What?**

Pluribus decides:

**Where?**

pluribussim decides:

**How does this host execute it?**

pluribusfabric decides:

**How do the endpoints communicate?**

That boundary is the foundation of the system.

---

# 41. Long-term picture

Eventually:

**Pluribus Machine**

- master workstation
- Linux build PC
- NVIDIA DRIVE AGX Thor
- GPU workstation
- AI worker
- CI machine

All contribute resources.

Applications see:

- executions
- services
- artifacts
- capabilities

not machines.

A RadarScape architecture could eventually request:

- UI execution
- radar decoder
- SAR processor
- OGM processor
- AI processor
- recording service
- visualization service

Pluribus places them across the available compute fabric.

Whether two processes happen to run on the same CPU, two different PCs, or a workstation and Thor becomes primarily a scheduling and transport decision rather than an application architecture decision.

That is the architectural goal.