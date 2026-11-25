#include "pluribus/core/execution.h"

#include "pluribus/core/paths.h"

#include <set>

namespace pluribus {
namespace {

constexpr std::size_t kMaxPackageNameLength = 128;
constexpr std::size_t kMaxPackageVersionLength = 64;
constexpr std::size_t kMaxLabelKeyLength = 128;
constexpr std::size_t kMaxClientPathLength = 1024;

bool is_ascii_alnum(char value) noexcept {
    return (value >= '0' && value <= '9') || (value >= 'a' && value <= 'z') ||
           (value >= 'A' && value <= 'Z');
}

bool is_package_name_component_char(char value) noexcept {
    return is_ascii_alnum(value) || value == '_' || value == '-';
}

bool valid_package_name_component(std::string_view component) noexcept {
    if (component.empty() || component.size() > 63) {
        return false;
    }
    if (!is_ascii_alnum(component.front()) || !is_ascii_alnum(component.back())) {
        return false;
    }
    for (const char value : component) {
        if (!is_package_name_component_char(value)) {
            return false;
        }
    }
    return true;
}

bool valid_package_name(std::string_view name) noexcept {
    if (name.empty() || name.size() > kMaxPackageNameLength) {
        return false;
    }

    std::size_t start = 0;
    while (start <= name.size()) {
        const auto end = name.find('.', start);
        const auto component = name.substr(
            start, end == std::string_view::npos ? name.size() - start : end - start);
        if (!valid_package_name_component(component)) {
            return false;
        }
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return true;
}

bool valid_package_version(std::string_view version) noexcept {
    if (version.empty() || version.size() > kMaxPackageVersionLength) {
        return false;
    }
    if (!is_ascii_alnum(version.front()) || !is_ascii_alnum(version.back())) {
        return false;
    }
    for (const char value : version) {
        if (!is_ascii_alnum(value) && value != '.' && value != '_' && value != '+' &&
            value != '-') {
            return false;
        }
    }
    return true;
}

bool valid_client_path(std::string_view path) noexcept {
    if (path.size() < 2 || path.size() > kMaxClientPathLength) {
        return false;
    }
    if (!path.starts_with("./") && !path.starts_with(".\\")) {
        return false;
    }
    if (path.find('\0') != std::string_view::npos) {
        return false;
    }

    std::size_t start = 0;
    while (start <= path.size()) {
        const auto end = path.find_first_of("/\\", start);
        const auto segment = path.substr(
            start, end == std::string_view::npos ? path.size() - start : end - start);
        if (segment == "..") {
            return false;
        }
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return path != "./" && path != ".\\";
}

bool contains_nul(std::string_view text) noexcept {
    return text.find('\0') != std::string_view::npos;
}

bool valid_environment_name(std::string_view name) noexcept {
    if (name.empty()) {
        return false;
    }
    const char first = name.front();
    if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') || first == '_')) {
        return false;
    }
    for (const char value : name) {
        if (!((value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
              (value >= '0' && value <= '9') || value == '_')) {
            return false;
        }
    }
    return true;
}

bool valid_label_key(std::string_view key) noexcept {
    if (key.empty() || key.size() > kMaxLabelKeyLength) {
        return false;
    }
    for (const unsigned char value : key) {
        if (value < 0x20 || value == 0x7F) {
            return false;
        }
    }
    return true;
}

} // namespace

std::optional<ExecutableRef> ExecutableRef::from_package(
    std::string name,
    std::optional<std::string> version) {
    if (!valid_package_name(name)) {
        return std::nullopt;
    }
    if (version && !valid_package_version(*version)) {
        return std::nullopt;
    }

    ExecutableRef result;
    result.kind_ = Kind::package;
    result.name_ = std::move(name);
    result.version_ = std::move(version);
    return result;
}

std::optional<ExecutableRef> ExecutableRef::from_artifact(ArtifactId id) {
    if (id.is_nil()) {
        return std::nullopt;
    }

    ExecutableRef result;
    result.kind_ = Kind::artifact;
    result.artifact_ = id;
    return result;
}

std::optional<ExecutableRef> ExecutableRef::from_client_file(std::string path) {
    if (!valid_client_path(path)) {
        return std::nullopt;
    }

    ExecutableRef result;
    result.kind_ = Kind::client_file;
    result.name_ = std::move(path);
    return result;
}

std::optional<ExecutableRef> ExecutableRef::parse(std::string_view text) {
    if (text.starts_with("artifact:")) {
        const auto artifact = ArtifactId::parse(text.substr(std::string_view("artifact:").size()));
        if (!artifact) {
            return std::nullopt;
        }
        return from_artifact(*artifact);
    }

    if (text.starts_with("./") || text.starts_with(".\\")) {
        return from_client_file(std::string(text));
    }

    const auto separator = text.find('@');
    if (separator == std::string_view::npos) {
        return from_package(std::string(text));
    }
    if (text.find('@', separator + 1) != std::string_view::npos) {
        return std::nullopt;
    }
    return from_package(
        std::string(text.substr(0, separator)),
        std::string(text.substr(separator + 1)));
}

bool ExecutableRef::valid() const noexcept {
    switch (kind_) {
    case Kind::package:
        return valid_package_name(name_) && (!version_ || valid_package_version(*version_));
    case Kind::artifact:
        return !artifact_.is_nil();
    case Kind::client_file:
        return valid_client_path(name_);
    }
    return false;
}

std::string_view ExecutableRef::package_name() const noexcept {
    return is_package() ? std::string_view(name_) : std::string_view{};
}

std::optional<std::string_view> ExecutableRef::package_version() const noexcept {
    if (!is_package() || !version_) {
        return std::nullopt;
    }
    return std::string_view(*version_);
}

std::string_view ExecutableRef::client_path() const noexcept {
    return is_client_file() ? std::string_view(name_) : std::string_view{};
}

std::string ExecutableRef::to_string() const {
    switch (kind_) {
    case Kind::package: {
        std::string result = name_;
        if (version_) {
            result.push_back('@');
            result += *version_;
        }
        return result;
    }
    case Kind::artifact:
        return "artifact:" + artifact_.to_string();
    case Kind::client_file:
        return name_;
    }
    return {};
}

Result<void> ExecutionSpec::validate() const {
    if (request_id.is_nil()) {
        return std::unexpected(
            make_error(ErrorCode::invalid_argument, "request_id must not be nil"));
    }
    if (!executable.valid()) {
        return std::unexpected(
            make_error(ErrorCode::invalid_argument, "executable reference is invalid"));
    }
    if (!working_directory.empty() && !is_sandbox_relative_path(working_directory)) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument, "working_directory must be sandbox-relative"));
    }

    std::set<std::string, std::less<>> input_paths;
    for (const auto& input : inputs) {
        if (input.artifact.is_nil()) {
            return std::unexpected(
                make_error(ErrorCode::invalid_argument, "input artifact must not be nil"));
        }
        if (auto result = validate_sandbox_relative_path(input.path); !result) {
            return result;
        }
        if (!input_paths.insert(input.path).second) {
            return std::unexpected(
                make_error(ErrorCode::invalid_argument, "duplicate input path"));
        }
    }

    std::set<std::string, std::less<>> output_paths;
    for (const auto& output : outputs) {
        if (auto result = validate_sandbox_relative_path(output.path); !result) {
            return result;
        }
        if (!output_paths.insert(output.path).second) {
            return std::unexpected(
                make_error(ErrorCode::invalid_argument, "duplicate output path"));
        }
    }

    for (const auto& argument : arguments) {
        if (contains_nul(argument)) {
            return std::unexpected(make_error(
                ErrorCode::invalid_argument, "arguments must not contain NUL bytes"));
        }
    }

    for (const auto& [name, value] : environment) {
        if (!valid_environment_name(name)) {
            return std::unexpected(
                make_error(ErrorCode::invalid_argument, "environment name is invalid"));
        }
        if (contains_nul(value)) {
            return std::unexpected(make_error(
                ErrorCode::invalid_argument, "environment value must not contain NUL bytes"));
        }
    }

    for (const auto& [key, value] : labels) {
        if (!valid_label_key(key)) {
            return std::unexpected(
                make_error(ErrorCode::invalid_argument, "label key is invalid"));
        }
        if (contains_nul(value)) {
            return std::unexpected(make_error(
                ErrorCode::invalid_argument, "label value must not contain NUL bytes"));
        }
    }

    if (resources.cpu && *resources.cpu < 0) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument, "cpu requirement must not be negative"));
    }
    if (constraints.node && constraints.node->is_nil()) {
        return std::unexpected(
            make_error(ErrorCode::invalid_argument, "explicit node must not be nil"));
    }
    if (constraints.abi_version && constraints.abi_version->empty()) {
        return std::unexpected(
            make_error(ErrorCode::invalid_argument, "abi_version constraint must not be empty"));
    }

    std::set<std::string, std::less<>> service_names;
    for (const auto& service : services) {
        if (service.empty()) {
            return std::unexpected(
                make_error(ErrorCode::invalid_argument, "service name must not be empty"));
        }
        if (!service_names.insert(service.to_string()).second) {
            return std::unexpected(
                make_error(ErrorCode::invalid_argument, "duplicate service name"));
        }
    }

    return {};
}

std::string_view to_string(ExecutionState state) {
    switch (state) {
    case ExecutionState::created:
        return "Created";
    case ExecutionState::queued:
        return "Queued";
    case ExecutionState::assigned:
        return "Assigned";
    case ExecutionState::materializing:
        return "Materializing";
    case ExecutionState::starting:
        return "Starting";
    case ExecutionState::running:
        return "Running";
    case ExecutionState::completed:
        return "Completed";
    case ExecutionState::failed:
        return "Failed";
    case ExecutionState::terminated:
        return "Terminated";
    case ExecutionState::lost:
        return "Lost";
    }
    return "Unknown";
}

std::optional<ExecutionState> parse_execution_state(std::string_view text) {
    if (text == "Created") {
        return ExecutionState::created;
    }
    if (text == "Queued") {
        return ExecutionState::queued;
    }
    if (text == "Assigned") {
        return ExecutionState::assigned;
    }
    if (text == "Materializing") {
        return ExecutionState::materializing;
    }
    if (text == "Starting") {
        return ExecutionState::starting;
    }
    if (text == "Running") {
        return ExecutionState::running;
    }
    if (text == "Completed") {
        return ExecutionState::completed;
    }
    if (text == "Failed") {
        return ExecutionState::failed;
    }
    if (text == "Terminated") {
        return ExecutionState::terminated;
    }
    if (text == "Lost") {
        return ExecutionState::lost;
    }
    return std::nullopt;
}

bool is_definitive_terminal(ExecutionState state) noexcept {
    return state == ExecutionState::completed || state == ExecutionState::failed ||
           state == ExecutionState::terminated;
}

bool is_terminal_observation(ExecutionState state) noexcept {
    return is_definitive_terminal(state) || state == ExecutionState::lost;
}

bool can_transition(ExecutionState from, ExecutionState to) noexcept {
    switch (from) {
    case ExecutionState::created:
        return to == ExecutionState::queued || to == ExecutionState::failed ||
               to == ExecutionState::terminated;
    case ExecutionState::queued:
        return to == ExecutionState::assigned || to == ExecutionState::failed ||
               to == ExecutionState::terminated;
    case ExecutionState::assigned:
        return to == ExecutionState::materializing || to == ExecutionState::failed ||
               to == ExecutionState::terminated || to == ExecutionState::lost;
    case ExecutionState::materializing:
        return to == ExecutionState::starting || to == ExecutionState::failed ||
               to == ExecutionState::terminated || to == ExecutionState::lost;
    case ExecutionState::starting:
        return to == ExecutionState::running || to == ExecutionState::failed ||
               to == ExecutionState::terminated || to == ExecutionState::lost;
    case ExecutionState::running:
        return to == ExecutionState::completed || to == ExecutionState::failed ||
               to == ExecutionState::terminated || to == ExecutionState::lost;
    case ExecutionState::lost:
        return to == ExecutionState::assigned || to == ExecutionState::materializing ||
               to == ExecutionState::starting || to == ExecutionState::running ||
               to == ExecutionState::completed || to == ExecutionState::failed ||
               to == ExecutionState::terminated;
    case ExecutionState::completed:
    case ExecutionState::failed:
    case ExecutionState::terminated:
        return false;
    }
    return false;
}

Result<void> validate_transition(ExecutionState from, ExecutionState to) {
    if (!can_transition(from, to)) {
        return std::unexpected(make_error(
            ErrorCode::conflict,
            "invalid execution transition from " + std::string(to_string(from)) + " to " +
                std::string(to_string(to))));
    }
    return {};
}

} // namespace pluribus
