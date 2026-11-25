#pragma once

#include <string_view>

#include "pluribus/core/error.h"

namespace pluribus {

// True for a portable, sandbox-relative logical path: a non-empty '/'
// separated path with no root, drive letter, backslash, "." or ".." segment.
// This is a validation rule, not filesystem virtualization.
bool is_sandbox_relative_path(std::string_view path) noexcept;

Result<void> validate_sandbox_relative_path(std::string_view path);

} // namespace pluribus
