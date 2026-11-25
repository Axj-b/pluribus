#pragma once

#include <compare>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace pluribus {

// Extensible dotted capability name, for example "gpu.cuda" or
// "device.radar.front". Case-sensitive and intentionally opaque.
class CapabilityName {
public:
    CapabilityName() = default;

    static std::optional<CapabilityName> parse(std::string_view text);

    bool empty() const noexcept { return value_.empty(); }
    std::string_view value() const noexcept { return value_; }
    std::string to_string() const { return value_; }

    friend bool operator==(const CapabilityName&, const CapabilityName&) = default;
    friend auto operator<=>(const CapabilityName&, const CapabilityName&) = default;

private:
    explicit CapabilityName(std::string value) : value_(std::move(value)) {}

    std::string value_;
};

// Hierarchical logical service name, for example "radarscape/sar/output".
// The complete interoperability grammar is still open; this implementation
// accepts slash-separated logical segments only and rejects host/port syntax.
class ServiceName {
public:
    ServiceName() = default;

    static std::optional<ServiceName> parse(std::string_view text);

    bool empty() const noexcept { return value_.empty(); }
    std::string_view value() const noexcept { return value_; }
    std::string to_string() const { return value_; }

    friend bool operator==(const ServiceName&, const ServiceName&) = default;
    friend auto operator<=>(const ServiceName&, const ServiceName&) = default;

private:
    explicit ServiceName(std::string value) : value_(std::move(value)) {}

    std::string value_;
};

} // namespace pluribus
