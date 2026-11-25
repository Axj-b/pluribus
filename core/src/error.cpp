#include "pluribus/core/error.h"

namespace pluribus {

std::string_view to_wire_code(ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::invalid_argument:
        return "INVALID_ARGUMENT";
    case ErrorCode::message_too_large:
        return "MESSAGE_TOO_LARGE";
    case ErrorCode::unsupported_version:
        return "UNSUPPORTED_VERSION";
    case ErrorCode::unsupported_feature:
        return "UNSUPPORTED_FEATURE";
    case ErrorCode::unauthorized:
        return "UNAUTHORIZED";
    case ErrorCode::not_found:
        return "NOT_FOUND";
    case ErrorCode::ambiguous:
        return "AMBIGUOUS";
    case ErrorCode::conflict:
        return "CONFLICT";
    case ErrorCode::unavailable:
        return "UNAVAILABLE";
    case ErrorCode::timeout:
        return "TIMEOUT";
    case ErrorCode::resource_exhausted:
        return "RESOURCE_EXHAUSTED";
    case ErrorCode::unschedulable:
        return "UNSCHEDULABLE";
    case ErrorCode::sequence_gap:
        return "SEQUENCE_GAP";
    case ErrorCode::resync_required:
        return "RESYNC_REQUIRED";
    case ErrorCode::artifact_missing:
        return "ARTIFACT_MISSING";
    case ErrorCode::integrity_error:
        return "INTEGRITY_ERROR";
    case ErrorCode::launch_failed:
        return "LAUNCH_FAILED";
    case ErrorCode::internal:
        return "INTERNAL";
    }
    return "INTERNAL";
}

} // namespace pluribus
