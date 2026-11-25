#include "pluribus/core/paths.h"

#include <string>

namespace pluribus {
namespace {

constexpr std::size_t kMaxSandboxPathLength = 1024;

bool has_control_character(std::string_view text) noexcept {
    for (const unsigned char value : text) {
        if (value < 0x20 || value == 0x7F) {
            return true;
        }
    }
    return false;
}

} // namespace

bool is_sandbox_relative_path(std::string_view path) noexcept {
    if (path.empty() || path.size() > kMaxSandboxPathLength) {
        return false;
    }
    if (path.front() == '/' || path.find('\\') != std::string_view::npos ||
        path.find(':') != std::string_view::npos) {
        return false;
    }
    if (has_control_character(path)) {
        return false;
    }

    std::size_t start = 0;
    while (start <= path.size()) {
        const auto end = path.find('/', start);
        const auto segment = path.substr(
            start, end == std::string_view::npos ? path.size() - start : end - start);
        if (segment.empty() || segment == "." || segment == "..") {
            return false;
        }
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return true;
}

Result<void> validate_sandbox_relative_path(std::string_view path) {
    if (!is_sandbox_relative_path(path)) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument,
            "path must be sandbox-relative and must not contain root, drive, traversal or "
            "backslash segments"));
    }
    return {};
}

} // namespace pluribus
