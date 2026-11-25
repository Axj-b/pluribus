---
title: "03. C++23 public API"
type: spec
id: spec-03
status: draft
tags: [spec, api, cpp]
last_reviewed: 2026-11-25
---

# 03. C++23 public API sketch

Status: illustrative, **non-final** C++23 design; names, ownership, async integration and ABI are not frozen. The V1 public library is C++23, in `pluribus::`; there is no stable C ABI yet. Concepts deliberately avoid C++ implementation types and host handles so a later C ABI and Python, C# or Rust bindings can wrap the same operations. A language-neutral [control protocol](02-node-master-protocol.md) remains independent of C++ layout. See [execution](01-execution-model.md), [services](04-service-model.md), [wire](05-wire-protocol.md), [runtime boundary](06-node-runtime-boundary.md), [repository](09-repository-cmake-structure.md), and [security](10-security-model.md).

## Surface and ownership

The public library supplies connect, submit, execution handles, observation/wait, separate stdout/stderr streaming, graceful termination/kill, and logical service creation/readiness/resolution/connection. Handles are opaque value-like identities backed by library-managed resources; copying a handle must not duplicate an execution. Destroying a handle or disconnecting does not implicitly terminate an execution. An operation's result distinguishes accepted control requests from confirmed execution outcomes. Use strong typed identifiers and portable error categories rather than raw host errors, file descriptors, PIDs, socket types, native paths, third-party async types or exposed implementation objects.

The following is conceptual pseudocode using C++23 syntax: declarations such as `task<T>` and the sketch types stand for public library types to be designed, not existing APIs or a promise of source/ABI compatibility.

```cpp
// NON-FINAL SKETCH; not a complete compilable header.
namespace pluribus {

struct Error;                 // portable code, diagnostic, optional context
template<class T> using Result = std::expected<T, Error>;
template<class T> class task; // library-defined awaitable, exact model TBD

struct ExecutionId;
struct NodeId;                // UUID, distinct from configurable node name
struct RequestId;             // client-generated and stable across retries
struct ExecutionSpec;         // see execution model; includes RequestId
struct ExecutionSnapshot;     // state, revision, node?, reason/exit?
struct LogChunk;              // stdout/stderr, timestamp, bytes, cursor/gap
struct ServiceName;
struct ServiceProvider;
struct ServiceResolution;    // logical identity and opaque routing recipe
struct ByteStream;
struct Duration;

class LogStream {
public:
    Result<std::optional<LogChunk>> next(); // EOF is nullopt; errors are distinct
    task<Result<std::optional<LogChunk>>> next_async();
};

class ExecutionHandle {
public:
    ExecutionId id() const;
    Result<ExecutionSnapshot> query() const;
    Result<ExecutionSnapshot> wait(Duration timeout) const;
    Result<LogStream> logs(/* stream selection, optional cursors, follow */) const;
    Result<void> terminate() const; // accepted, not necessarily stopped
    Result<void> kill() const;
    task<Result<ExecutionSnapshot>> wait_async(Duration timeout) const;
    task<Result<void>> terminate_async() const;
};

class Client {
public:
    static Result<Client> connect(/* logical master configuration */);
    static task<Result<Client>> connect_async(/* configuration */);
    Result<ExecutionHandle> submit(const ExecutionSpec& spec);
    task<Result<ExecutionHandle>> submit_async(const ExecutionSpec& spec);
    Result<ExecutionHandle> execution(ExecutionId id);
    Result<ServiceResolution> resolve(ServiceName name, Duration timeout);
    Result<ByteStream> connect_service(const ServiceResolution& resolution);
    task<Result<ServiceResolution>> resolve_async(ServiceName name, Duration timeout);
    task<Result<ByteStream>> connect_service_async(const ServiceResolution& resolution);
};

// Available inside an execution through its local node/runtime endpoint.
class ServiceContext {
public:
    static Result<ServiceContext> connect_local();
    Result<ServiceProvider> create(ServiceName name); // created, not yet READY
};
class ServiceProvider {
public:
    Result<void> ready(); // after initialization; exposes provider to resolvers
    Result<ByteStream> accept();
    Result<void> close();
};
} // namespace pluribus
```

`ByteStream` represents a reliable, ordered bidirectional byte stream with read, write and close operations and async equivalents; it is not a native socket. `ServiceProvider::accept` receives connected streams, not a physical network address. Exact buffer and stream APIs are deliberately omitted from the sketch. Logical paths in specs are sandbox-relative; V1 does not promise path virtualization, and platform-specific interoperability should remain outside the core API.

## Usage sketch

```cpp
// NON-FINAL SKETCH: construction/helpers are illustrative.
auto client = pluribus::Client::connect(config).value();
pluribus::ExecutionSpec spec = make_spec("hello");
spec.request_id = make_request_id(); // keep this value for any retry
auto handle = client.submit(spec).value();
auto logs = handle.logs(/* both streams, retained history, follow */).value();
// Consume chunks asynchronously while waiting; do not block log draining.
auto result = handle.wait(timeout);
// Inspect expected, state, exit reason and possible Lost; do not assume success.

auto provider = pluribus::ServiceContext::connect_local().value()
                    .create(service_name("radarscape/sar/output")).value();
initialize_application();
provider.ready().value();
auto resolution = client.resolve(service_name("radarscape/sar/output"), timeout);
// Check resolution and connect_service; re-resolve if a provider disappeared.
```

The `.value()` calls are shorthand for the sketch only. Production callers must handle errors. Service provider creation, accept and marking ready happen in the provider execution; consumer resolution/connect may happen in another process. A submitted execution can declare names, but only the running provider can establish readiness.

## Async, cancellation and errors

- Sync forms block the calling thread only. Async forms support coroutine-based event-driven networking without exposing an I/O framework, executor or platform handle. They return equivalent result/error semantics. The library must specify executor ownership, operation lifetime, cancellation and thread safety before an API is finalized.
- Cancelling an API wait/stream or timing out stops that local operation; it does not stop the execution. To stop it, explicitly call terminate or kill. CLI foreground interrupt may issue terminate; detach simply stops waiting.
- Public fallible operations return `std::expected<T, Error>` (or an async result containing it). `Error` supplies a stable portable category/code, diagnostic and optional context; remote application failure is an execution outcome, distinct from failure to query it. Timeouts leave mutation outcome unknown; retry submission with the same request ID and identical spec.
- `wait` returns when the execution is definitively terminal **or Lost**. Lost is an unknown, reconcilable outcome, not proof of process death. Query/watch again for later revisions. A log subscription reports retention gaps; stream reads must tolerate disconnection.
- Compatibility uses Pluribus ABI plus CPU architecture, not host OS as a scheduler field. This is a target contract whose native portability mechanism must be prototyped, not a claim that arbitrary native binaries already run on every host.

## Bindings and stability

Keep public values self-contained, no templates involving third-party libraries, no ownership tied to undocumented implementation layouts, and no exceptions as the only way to report operational failures. C++23 APIs can use `std::expected` and library-owned opaque types; a future C wrapper will translate these into explicit handles, buffers, error codes, destruction functions and asynchronous callbacks. This is preparation, **not** a stable C ABI or a guarantee of binary compatibility across C++ toolchains. See [wire stability](05-wire-protocol.md#compatibility-and-stable-contract).

## Open issues

- Final type names, request ID generation and client identity, public construction/validation of specs, async executor/cancellation rules, thread safety and handle ownership need implementation design.
- Exact log cursor, byte-buffer and stream shutdown APIs, error-code representation and C++ source/binary compatibility policy remain undecided.

## Knowledge base links

- Map: [Specification map](../maps/specifications.md)
- Notes: [CLI](../notes/cli.md), [Execution](../notes/execution.md)
- Decisions: [Decision records](../decisions/README.md), [Decision map](../maps/decisions.md)
- Open questions: [Open questions register](../questions/README.md)
