#pragma once

#include <chrono>
#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace pluribus {

using CpuMillicores = std::int64_t;
using MemoryBytes = std::uint64_t;
using LoadBasisPoints = std::uint32_t;
using Revision = std::uint64_t;
using Timestamp = std::chrono::system_clock::time_point;

inline constexpr LoadBasisPoints kFullLoadBasisPoints = 10'000;

enum class CpuArchitecture : std::uint8_t {
    x86_64,
    arm64,
};

std::string_view to_string(CpuArchitecture architecture);
std::optional<CpuArchitecture> parse_cpu_architecture(std::string_view text);

// Runtime ABI version, for example "pluribus-v1". Separate from the wire
// protocol version and the package format version.
class AbiVersion {
public:
    AbiVersion() = default;

    static std::optional<AbiVersion> parse(std::string_view text);

    bool empty() const noexcept { return value_.empty(); }
    std::string_view value() const noexcept { return value_; }
    std::string to_string() const { return value_; }

    friend bool operator==(const AbiVersion&, const AbiVersion&) = default;
    friend auto operator<=>(const AbiVersion&, const AbiVersion&) = default;

private:
    explicit AbiVersion(std::string value) : value_(std::move(value)) {}

    std::string value_;
};

// A node compatibility key: Pluribus ABI plus CPU architecture. Host OS is
// deliberately not part of this key.
struct CompatibilityKey {
    AbiVersion abi;
    CpuArchitecture architecture = CpuArchitecture::x86_64;

    std::string to_string() const;

    friend bool operator==(const CompatibilityKey&, const CompatibilityKey&) = default;
    friend auto operator<=>(const CompatibilityKey&, const CompatibilityKey&) = default;
};

} // namespace pluribus
