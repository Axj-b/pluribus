#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace pluribus {

// RFC 4122-style 128-bit identifier. The text form is lower-case
// "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
class Uuid {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    static constexpr std::size_t text_length = 36;

    Uuid() = default;
    explicit Uuid(Bytes bytes) noexcept : bytes_(bytes) {}

    static Uuid generate();
    static std::optional<Uuid> parse(std::string_view text) noexcept;
    static Uuid nil() noexcept { return Uuid{}; }

    bool is_nil() const noexcept;
    const Bytes& bytes() const noexcept { return bytes_; }
    std::string to_string() const;

    friend bool operator==(const Uuid&, const Uuid&) = default;
    friend auto operator<=>(const Uuid&, const Uuid&) = default;

private:
    Bytes bytes_{};
};

// Distinct identity types built on Uuid. They are intentionally not
// interchangeable, even though they share a representation.
template <class Tag>
class UuidId {
public:
    UuidId() = default;
    explicit UuidId(Uuid value) noexcept : value_(value) {}

    static UuidId generate() { return UuidId(Uuid::generate()); }
    static std::optional<UuidId> parse(std::string_view text) noexcept {
        const auto parsed = Uuid::parse(text);
        if (!parsed) {
            return std::nullopt;
        }
        return UuidId(*parsed);
    }

    bool is_nil() const noexcept { return value_.is_nil(); }
    const Uuid& value() const noexcept { return value_; }
    std::string to_string() const { return value_.to_string(); }

    friend bool operator==(const UuidId&, const UuidId&) = default;
    friend auto operator<=>(const UuidId&, const UuidId&) = default;

private:
    Uuid value_{};
};

struct NodeIdTag;
struct ExecutionIdTag;
struct RequestIdTag;
struct AssignmentIdTag;
struct ServiceIdTag;
struct ServiceProviderIdTag;

using NodeId = UuidId<NodeIdTag>;
using ExecutionId = UuidId<ExecutionIdTag>;
using RequestId = UuidId<RequestIdTag>;
using AssignmentId = UuidId<AssignmentIdTag>;
using ServiceId = UuidId<ServiceIdTag>;
using ServiceProviderId = UuidId<ServiceProviderIdTag>;

// SHA-256 content digest. Immutable content identity, never a file name.
class Sha256Digest {
public:
    using Bytes = std::array<std::uint8_t, 32>;

    static constexpr std::size_t hex_length = 64;

    Sha256Digest() = default;
    explicit Sha256Digest(Bytes bytes) noexcept : bytes_(bytes) {}

    // Parses exactly 64 lower-case hexadecimal digits.
    static std::optional<Sha256Digest> parse_hex(std::string_view text) noexcept;
    static Sha256Digest from_bytes(Bytes bytes) noexcept { return Sha256Digest(bytes); }

    bool is_nil() const noexcept;
    const Bytes& bytes() const noexcept { return bytes_; }
    std::string to_hex() const;

    friend bool operator==(const Sha256Digest&, const Sha256Digest&) = default;
    friend auto operator<=>(const Sha256Digest&, const Sha256Digest&) = default;

private:
    Bytes bytes_{};
};

// Canonical text form: "sha256:" followed by 64 lower-case hexadecimal digits.
class ArtifactId {
public:
    static constexpr std::string_view scheme = "sha256:";

    ArtifactId() = default;
    explicit ArtifactId(Sha256Digest digest) noexcept : digest_(digest) {}

    static std::optional<ArtifactId> parse(std::string_view text) noexcept;
    static ArtifactId from_digest(Sha256Digest digest) noexcept { return ArtifactId(digest); }

    bool is_nil() const noexcept { return digest_.is_nil(); }
    const Sha256Digest& digest() const noexcept { return digest_; }
    std::string to_string() const;

    friend bool operator==(const ArtifactId&, const ArtifactId&) = default;
    friend auto operator<=>(const ArtifactId&, const ArtifactId&) = default;

private:
    Sha256Digest digest_{};
};

} // namespace pluribus
