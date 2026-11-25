#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "pluribus/core/ids.h"

namespace pluribus {

// Portable error categories. Host errno/GetLastError values never cross this
// boundary as API types.
enum class ErrorCode : std::uint8_t {
    invalid_argument,
    message_too_large,
    unsupported_version,
    unsupported_feature,
    unauthorized,
    not_found,
    ambiguous,
    conflict,
    unavailable,
    timeout,
    resource_exhausted,
    unschedulable,
    sequence_gap,
    resync_required,
    artifact_missing,
    integrity_error,
    launch_failed,
    internal,
};

std::string_view to_wire_code(ErrorCode code) noexcept;

struct Error {
    ErrorCode code = ErrorCode::internal;
    std::string diagnostic;
    std::string component;
    std::optional<NodeId> node;
    std::optional<ExecutionId> execution;
};

inline Error make_error(ErrorCode code, std::string diagnostic) {
    return Error{code, std::move(diagnostic), {}, std::nullopt, std::nullopt};
}

template <class T>
using Result = std::expected<T, Error>;

} // namespace pluribus
