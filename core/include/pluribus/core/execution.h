#pragma once

#include <compare>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "pluribus/core/error.h"
#include "pluribus/core/ids.h"
#include "pluribus/core/names.h"
#include "pluribus/core/resources.h"

namespace pluribus {

// Explicit executable source from docs/spec/01-execution-model.md:
//   hello                   -> package name
//   hello@1.4.2             -> exact package version
//   artifact:sha256:<digest> -> immutable content
//   ./hello                 -> client-local file, uploaded by the client
// There is no node-local PATH lookup or implicit source precedence.
class ExecutableRef {
public:
    enum class Kind : std::uint8_t {
        package,
        artifact,
        client_file,
    };

    ExecutableRef() = default;

    static std::optional<ExecutableRef> parse(std::string_view text);
    static std::optional<ExecutableRef> from_package(
        std::string name,
        std::optional<std::string> version = std::nullopt);
    static std::optional<ExecutableRef> from_artifact(ArtifactId id);
    static std::optional<ExecutableRef> from_client_file(std::string path);

    Kind kind() const noexcept { return kind_; }
    bool is_package() const noexcept { return kind_ == Kind::package; }
    bool is_artifact() const noexcept { return kind_ == Kind::artifact; }
    bool is_client_file() const noexcept { return kind_ == Kind::client_file; }
    bool valid() const noexcept;

    std::string_view package_name() const noexcept;
    std::optional<std::string_view> package_version() const noexcept;
    const ArtifactId& artifact() const noexcept { return artifact_; }
    std::string_view client_path() const noexcept;

    std::string to_string() const;

    friend bool operator==(const ExecutableRef&, const ExecutableRef&) = default;
    friend auto operator<=>(const ExecutableRef&, const ExecutableRef&) = default;

private:
    Kind kind_ = Kind::package;
    std::string name_;
    std::optional<std::string> version_;
    ArtifactId artifact_;
};

enum class ExecutionLifetime : std::uint8_t {
    finite,
    persistent,
};

// Ownership is independent of a client connection by default.
enum class OwnerLifetime : std::uint8_t {
    independent,
    client,
};

enum class ArtifactAccess : std::uint8_t {
    read_only,
    read_write,
};

struct InputSpec {
    ArtifactId artifact;
    std::string path;
    ArtifactAccess access = ArtifactAccess::read_only;

    friend bool operator==(const InputSpec&, const InputSpec&) = default;
    friend auto operator<=>(const InputSpec&, const InputSpec&) = default;
};

struct OutputSpec {
    std::string path;

    friend bool operator==(const OutputSpec&, const OutputSpec&) = default;
    friend auto operator<=>(const OutputSpec&, const OutputSpec&) = default;
};

// An absent value means unknown, not zero and not an implicit default.
struct ResourceRequirements {
    std::optional<CpuMillicores> cpu;
    std::optional<MemoryBytes> memory;

    friend bool operator==(const ResourceRequirements&, const ResourceRequirements&) = default;
};

struct SchedulingConstraints {
    std::optional<CpuArchitecture> architecture;
    std::optional<AbiVersion> abi_version;
    std::set<CapabilityName, std::less<>> required_capabilities;
    std::set<CapabilityName, std::less<>> preferred_capabilities;
    std::set<CapabilityName, std::less<>> required_devices;
    std::optional<NodeId> node;

    friend bool operator==(const SchedulingConstraints&, const SchedulingConstraints&) = default;
};

// Immutable after acceptance. Fields are logical model values; wire encoding
// is separate and not yet frozen.
struct ExecutionSpec {
    RequestId request_id;
    ExecutableRef executable;
    std::vector<std::string> arguments;
    std::map<std::string, std::string, std::less<>> environment;
    std::string working_directory;
    std::vector<InputSpec> inputs;
    std::vector<OutputSpec> outputs;
    ExecutionLifetime lifetime = ExecutionLifetime::finite;
    OwnerLifetime owner_lifetime = OwnerLifetime::independent;
    std::vector<ServiceName> services;
    ResourceRequirements resources;
    SchedulingConstraints constraints;
    std::map<std::string, std::string, std::less<>> labels;

    // Validates model-level invariants, including sandbox-relative paths.
    // Master policy and availability are not checked here.
    Result<void> validate() const;
};

enum class ExecutionState : std::uint8_t {
    created,
    queued,
    assigned,
    materializing,
    starting,
    running,
    completed,
    failed,
    terminated,
    lost,
};

std::string_view to_string(ExecutionState state);
std::optional<ExecutionState> parse_execution_state(std::string_view text);

bool is_definitive_terminal(ExecutionState state) noexcept;
bool is_terminal_observation(ExecutionState state) noexcept;
bool can_transition(ExecutionState from, ExecutionState to) noexcept;
Result<void> validate_transition(ExecutionState from, ExecutionState to);

struct ExecutionOutcome {
    std::optional<std::int32_t> exit_code;
    std::optional<std::int32_t> signal;

    friend bool operator==(const ExecutionOutcome&, const ExecutionOutcome&) = default;
};

struct ExecutionSnapshot {
    ExecutionId id;
    ExecutionState state = ExecutionState::created;
    Revision revision = 0;
    std::optional<NodeId> node;
    std::optional<ExecutionOutcome> outcome;
    std::string reason;
};

} // namespace pluribus
