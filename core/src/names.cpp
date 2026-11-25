#include "pluribus/core/names.h"

#include <vector>

namespace pluribus {
namespace {

constexpr std::size_t kMaxNameLength = 255;
constexpr std::size_t kMaxSegmentLength = 63;

bool is_ascii_alnum(char value) noexcept {
    return (value >= '0' && value <= '9') || (value >= 'a' && value <= 'z') ||
           (value >= 'A' && value <= 'Z');
}

bool is_component_char(char value) noexcept {
    return is_ascii_alnum(value) || value == '_' || value == '-';
}

// A dot-separated component such as "cuda", "x86_64" or "radar-front".
bool valid_component(std::string_view component) noexcept {
    if (component.empty() || component.size() > kMaxSegmentLength) {
        return false;
    }
    if (!is_ascii_alnum(component.front()) || !is_ascii_alnum(component.back())) {
        return false;
    }
    for (const char value : component) {
        if (!is_component_char(value)) {
            return false;
        }
    }
    return true;
}

// Splits on dot and validates every part. Rejects empty parts like "gpu..cuda".
bool valid_dotted(std::string_view text) noexcept {
    std::size_t start = 0;
    while (start <= text.size()) {
        const auto end = text.find('.', start);
        const auto part = text.substr(start, end == std::string_view::npos ? text.size() - start
                                                                          : end - start);
        if (!valid_component(part)) {
            return false;
        }
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return true;
}

bool valid_capability(std::string_view text) noexcept {
    return !text.empty() && text.size() <= kMaxNameLength && valid_dotted(text);
}

bool valid_service(std::string_view text) noexcept {
    if (text.empty() || text.size() > kMaxNameLength) {
        return false;
    }
    if (text.front() == '/' || text.back() == '/') {
        return false;
    }

    std::size_t start = 0;
    while (start <= text.size()) {
        const auto end = text.find('/', start);
        const auto segment = text.substr(
            start, end == std::string_view::npos ? text.size() - start : end - start);
        if (!valid_dotted(segment)) {
            return false;
        }
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return true;
}

} // namespace

std::optional<CapabilityName> CapabilityName::parse(std::string_view text) {
    if (!valid_capability(text)) {
        return std::nullopt;
    }
    return CapabilityName(std::string(text));
}

std::optional<ServiceName> ServiceName::parse(std::string_view text) {
    if (!valid_service(text)) {
        return std::nullopt;
    }
    return ServiceName(std::string(text));
}

} // namespace pluribus
