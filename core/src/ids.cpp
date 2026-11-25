#include "pluribus/core/ids.h"

#include <atomic>
#include <chrono>
#include <random>

namespace pluribus {
namespace {

constexpr char kHexDigits[] = "0123456789abcdef";

int hex_value(char value) noexcept {
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    if (value >= 'A' && value <= 'F') {
        return value - 'A' + 10;
    }
    return -1;
}

int lower_hex_value(char value) noexcept {
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    return -1;
}

} // namespace

Uuid Uuid::generate() {
    static std::atomic<std::uint64_t> counter{0};

    std::random_device random_device;
    std::uint64_t seed = (static_cast<std::uint64_t>(random_device()) << 32) ^
                         static_cast<std::uint64_t>(random_device());
    seed ^= static_cast<std::uint64_t>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    seed ^= counter.fetch_add(1, std::memory_order_relaxed) * 0x9E3779B97F4A7C15ULL;

    std::mt19937_64 engine(seed);
    const std::uint64_t first = engine();
    const std::uint64_t second = engine();

    Bytes bytes{};
    for (std::size_t index = 0; index < 8; ++index) {
        bytes[index] = static_cast<std::uint8_t>(first >> (index * 8));
        bytes[index + 8] = static_cast<std::uint8_t>(second >> (index * 8));
    }

    // RFC 4122 version 4 and variant bits.
    bytes[6] = static_cast<std::uint8_t>((bytes[6] & 0x0F) | 0x40);
    bytes[8] = static_cast<std::uint8_t>((bytes[8] & 0x3F) | 0x80);

    return Uuid(bytes);
}

std::optional<Uuid> Uuid::parse(std::string_view text) noexcept {
    if (text.size() != text_length) {
        return std::nullopt;
    }

    Bytes bytes{};
    std::size_t byte_index = 0;
    for (std::size_t index = 0; index < text_length;) {
        if (index == 8 || index == 13 || index == 18 || index == 23) {
            if (text[index] != '-') {
                return std::nullopt;
            }
            ++index;
            continue;
        }

        if (index + 1 >= text_length) {
            return std::nullopt;
        }
        const int high = hex_value(text[index]);
        const int low = hex_value(text[index + 1]);
        if (high < 0 || low < 0 || byte_index >= bytes.size()) {
            return std::nullopt;
        }
        bytes[byte_index++] = static_cast<std::uint8_t>((high << 4) | low);
        index += 2;
    }

    if (byte_index != bytes.size()) {
        return std::nullopt;
    }
    return Uuid(bytes);
}

bool Uuid::is_nil() const noexcept {
    for (const auto byte : bytes_) {
        if (byte != 0) {
            return false;
        }
    }
    return true;
}

std::string Uuid::to_string() const {
    std::string result(text_length, '-');
    std::size_t output = 0;
    for (std::size_t index = 0; index < bytes_.size(); ++index) {
        if (index == 4 || index == 6 || index == 8 || index == 10) {
            ++output;
        }
        result[output++] = kHexDigits[bytes_[index] >> 4];
        result[output++] = kHexDigits[bytes_[index] & 0x0F];
    }
    return result;
}

std::optional<Sha256Digest> Sha256Digest::parse_hex(std::string_view text) noexcept {
    if (text.size() != hex_length) {
        return std::nullopt;
    }

    Bytes bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        const int high = lower_hex_value(text[index * 2]);
        const int low = lower_hex_value(text[index * 2 + 1]);
        if (high < 0 || low < 0) {
            return std::nullopt;
        }
        bytes[index] = static_cast<std::uint8_t>((high << 4) | low);
    }
    return Sha256Digest(bytes);
}

bool Sha256Digest::is_nil() const noexcept {
    for (const auto byte : bytes_) {
        if (byte != 0) {
            return false;
        }
    }
    return true;
}

std::string Sha256Digest::to_hex() const {
    std::string result(hex_length, '0');
    for (std::size_t index = 0; index < bytes_.size(); ++index) {
        result[index * 2] = kHexDigits[bytes_[index] >> 4];
        result[index * 2 + 1] = kHexDigits[bytes_[index] & 0x0F];
    }
    return result;
}

std::optional<ArtifactId> ArtifactId::parse(std::string_view text) noexcept {
    if (!text.starts_with(scheme)) {
        return std::nullopt;
    }
    const auto digest = Sha256Digest::parse_hex(text.substr(scheme.size()));
    if (!digest) {
        return std::nullopt;
    }
    return ArtifactId(*digest);
}

std::string ArtifactId::to_string() const {
    return std::string(scheme) + digest_.to_hex();
}

} // namespace pluribus
