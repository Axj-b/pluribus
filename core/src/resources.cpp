#include "pluribus/core/resources.h"

namespace pluribus {
namespace {

bool is_ascii_alnum(char value) noexcept {
    return (value >= '0' && value <= '9') || (value >= 'a' && value <= 'z') ||
           (value >= 'A' && value <= 'Z');
}

} // namespace

std::string_view to_string(CpuArchitecture architecture) {
    switch (architecture) {
    case CpuArchitecture::x86_64:
        return "x86_64";
    case CpuArchitecture::arm64:
        return "arm64";
    }
    return "unknown";
}

std::optional<CpuArchitecture> parse_cpu_architecture(std::string_view text) {
    if (text == "x86_64") {
        return CpuArchitecture::x86_64;
    }
    if (text == "arm64") {
        return CpuArchitecture::arm64;
    }
    return std::nullopt;
}

std::optional<AbiVersion> AbiVersion::parse(std::string_view text) {
    if (text.empty() || text.size() > 64) {
        return std::nullopt;
    }
    if (!is_ascii_alnum(text.front()) || !is_ascii_alnum(text.back())) {
        return std::nullopt;
    }
    for (const char value : text) {
        if (!is_ascii_alnum(value) && value != '.' && value != '_' && value != '-') {
            return std::nullopt;
        }
    }
    return AbiVersion(std::string(text));
}

std::string CompatibilityKey::to_string() const {
    std::string result = abi.to_string();
    result.push_back('-');
    result += pluribus::to_string(architecture);
    return result;
}

} // namespace pluribus
