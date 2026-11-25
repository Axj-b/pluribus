#include "pluribus/fabric/handshake.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace pluribus::fabric {
namespace {

constexpr std::size_t kMaxIdentityBytes = 128;
constexpr std::size_t kMaxJoinTokenBytes = 4096;
constexpr std::size_t kMaxRejectDiagnosticBytes = 1024;
constexpr std::size_t kMaxVersions = 8;
constexpr std::size_t kSessionIdBytes = 16;

Error invalid(std::string diagnostic) {
    return make_error(ErrorCode::invalid_argument, std::move(diagnostic));
}

class Writer {
public:
    void u8(std::uint8_t value) {
        bytes_.push_back(static_cast<std::byte>(value));
    }

    void u16(std::uint16_t value) {
        bytes_.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
        bytes_.push_back(static_cast<std::byte>(value & 0xFF));
    }

    void u32(std::uint32_t value) {
        for (int shift = 24; shift >= 0; shift -= 8) {
            bytes_.push_back(static_cast<std::byte>((value >> shift) & 0xFF));
        }
    }

    void u64(std::uint64_t value) {
        for (int shift = 56; shift >= 0; shift -= 8) {
            bytes_.push_back(static_cast<std::byte>((value >> shift) & 0xFF));
        }
    }

    void bytes(std::span<const std::byte> value) {
        bytes_.insert(bytes_.end(), value.begin(), value.end());
    }

    void raw(std::span<const std::uint8_t> value) {
        for (const auto byte : value) {
            bytes_.push_back(static_cast<std::byte>(byte));
        }
    }

    void text(std::string_view value) {
        bytes(std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(value.data()), value.size()));
    }

    std::vector<std::byte> take() && { return std::move(bytes_); }

private:
    std::vector<std::byte> bytes_;
};

class Reader {
public:
    explicit Reader(std::span<const std::byte> bytes) : bytes_(bytes) {}

    bool u8(std::uint8_t& value) {
        if (remaining() < 1) {
            return false;
        }
        value = std::to_integer<std::uint8_t>(bytes_[offset_++]);
        return true;
    }

    bool u16(std::uint16_t& value) {
        if (remaining() < 2) {
            return false;
        }
        value = static_cast<std::uint16_t>(
            (std::to_integer<std::uint16_t>(bytes_[offset_]) << 8) |
            std::to_integer<std::uint16_t>(bytes_[offset_ + 1]));
        offset_ += 2;
        return true;
    }

    bool u32(std::uint32_t& value) {
        if (remaining() < 4) {
            return false;
        }
        value = (std::to_integer<std::uint32_t>(bytes_[offset_]) << 24) |
                (std::to_integer<std::uint32_t>(bytes_[offset_ + 1]) << 16) |
                (std::to_integer<std::uint32_t>(bytes_[offset_ + 2]) << 8) |
                std::to_integer<std::uint32_t>(bytes_[offset_ + 3]);
        offset_ += 4;
        return true;
    }

    bool u64(std::uint64_t& value) {
        if (remaining() < 8) {
            return false;
        }
        value = 0;
        for (std::size_t index = 0; index < 8; ++index) {
            value = (value << 8) | std::to_integer<std::uint64_t>(bytes_[offset_ + index]);
        }
        offset_ += 8;
        return true;
    }

    bool bytes(std::size_t count, std::span<const std::byte>& value) {
        if (remaining() < count) {
            return false;
        }
        value = bytes_.subspan(offset_, count);
        offset_ += count;
        return true;
    }

    bool text(std::size_t count, std::string& value) {
        if (count == 0) {
            value.clear();
            return true;
        }
        std::span<const std::byte> raw;
        if (!bytes(count, raw)) {
            return false;
        }
        value.assign(
            reinterpret_cast<const char*>(raw.data()),
            reinterpret_cast<const char*>(raw.data()) + raw.size());
        return true;
    }

    bool finished() const noexcept { return offset_ == bytes_.size(); }

private:
    std::size_t remaining() const noexcept { return bytes_.size() - offset_; }

    std::span<const std::byte> bytes_;
    std::size_t offset_ = 0;
};

Uuid session_uuid(std::span<const std::byte> raw) {
    Uuid::Bytes bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        bytes[index] = std::to_integer<std::uint8_t>(raw[index]);
    }
    return Uuid(bytes);
}

bool valid_role(SessionRole role) noexcept {
    return role == SessionRole::node || role == SessionRole::client;
}

bool valid_reject_code(HandshakeRejectCode code) noexcept {
    switch (code) {
    case HandshakeRejectCode::unsupported_version:
    case HandshakeRejectCode::unsupported_feature:
    case HandshakeRejectCode::unauthorized:
    case HandshakeRejectCode::invalid_argument:
    case HandshakeRejectCode::internal:
        return true;
    }
    return false;
}

} // namespace

ErrorCode to_error_code(HandshakeRejectCode code) noexcept {
    switch (code) {
    case HandshakeRejectCode::unsupported_version:
        return ErrorCode::unsupported_version;
    case HandshakeRejectCode::unsupported_feature:
        return ErrorCode::unsupported_feature;
    case HandshakeRejectCode::unauthorized:
        return ErrorCode::unauthorized;
    case HandshakeRejectCode::invalid_argument:
        return ErrorCode::invalid_argument;
    case HandshakeRejectCode::internal:
        return ErrorCode::internal;
    }
    return ErrorCode::internal;
}

Result<std::vector<std::byte>> encode_hello(const Hello& hello) {
    if (!valid_role(hello.role)) {
        return std::unexpected(invalid("hello role is invalid"));
    }
    if (hello.versions.empty() || hello.versions.size() > kMaxVersions) {
        return std::unexpected(invalid("hello version list is empty or too large"));
    }
    for (const auto& version : hello.versions) {
        if (version.major == 0) {
            return std::unexpected(invalid("bootstrap version cannot be advertised"));
        }
    }
    if (hello.identity.empty() || hello.identity.size() > kMaxIdentityBytes) {
        return std::unexpected(invalid("hello identity is empty or too long"));
    }
    if (hello.join_token && hello.join_token->size() > kMaxJoinTokenBytes) {
        return std::unexpected(invalid("hello join token is too long"));
    }
    if (hello.prior_session && hello.prior_session->is_nil()) {
        return std::unexpected(invalid("hello prior session is nil"));
    }

    Writer writer;
    writer.u8(static_cast<std::uint8_t>(hello.role));
    writer.u8(static_cast<std::uint8_t>(hello.versions.size()));
    for (const auto& version : hello.versions) {
        writer.u8(version.major);
        writer.u8(version.minor);
    }
    writer.u64(hello.features);
    writer.u32(static_cast<std::uint32_t>(hello.identity.size()));
    writer.text(hello.identity);
    writer.u16(static_cast<std::uint16_t>(hello.join_token ? hello.join_token->size() : 0));
    if (hello.join_token) {
        writer.text(*hello.join_token);
    }
    writer.u8(hello.prior_session ? 1 : 0);
    if (hello.prior_session) {
        writer.raw(hello.prior_session->value().bytes());
    }
    return std::move(writer).take();
}

Result<Hello> decode_hello(std::span<const std::byte> payload) {
    Reader reader(payload);
    std::uint8_t role = 0;
    std::uint8_t version_count = 0;
    if (!reader.u8(role) || !reader.u8(version_count)) {
        return std::unexpected(invalid("truncated hello header"));
    }
    if (!valid_role(static_cast<SessionRole>(role))) {
        return std::unexpected(invalid("hello role is invalid"));
    }
    if (version_count == 0 || version_count > kMaxVersions) {
        return std::unexpected(invalid("hello version list is empty or too large"));
    }

    Hello hello;
    hello.role = static_cast<SessionRole>(role);
    hello.versions.reserve(version_count);
    for (std::uint8_t index = 0; index < version_count; ++index) {
        std::uint8_t major = 0;
        std::uint8_t minor = 0;
        if (!reader.u8(major) || !reader.u8(minor)) {
            return std::unexpected(invalid("truncated hello version list"));
        }
        if (major == 0) {
            return std::unexpected(invalid("bootstrap version cannot be advertised"));
        }
        hello.versions.push_back(ProtocolVersion{major, minor});
    }

    if (!reader.u64(hello.features)) {
        return std::unexpected(invalid("truncated hello features"));
    }

    std::uint32_t identity_length = 0;
    if (!reader.u32(identity_length) || identity_length == 0 ||
        identity_length > kMaxIdentityBytes) {
        return std::unexpected(invalid("hello identity length is invalid"));
    }
    if (!reader.text(identity_length, hello.identity)) {
        return std::unexpected(invalid("truncated hello identity"));
    }

    std::uint16_t token_length = 0;
    if (!reader.u16(token_length) || token_length > kMaxJoinTokenBytes) {
        return std::unexpected(invalid("hello join token length is invalid"));
    }
    if (token_length != 0) {
        std::string token;
        if (!reader.text(token_length, token)) {
            return std::unexpected(invalid("truncated hello join token"));
        }
        hello.join_token = std::move(token);
    }

    std::uint8_t has_prior = 0;
    if (!reader.u8(has_prior) || has_prior > 1) {
        return std::unexpected(invalid("hello prior-session flag is invalid"));
    }
    if (has_prior == 1) {
        std::span<const std::byte> raw;
        if (!reader.bytes(kSessionIdBytes, raw)) {
            return std::unexpected(invalid("truncated hello prior session"));
        }
        SessionId id(session_uuid(raw));
        if (id.is_nil()) {
            return std::unexpected(invalid("hello prior session is nil"));
        }
        hello.prior_session = id;
    }

    if (!reader.finished()) {
        return std::unexpected(invalid("trailing bytes in hello payload"));
    }
    return hello;
}

Result<std::vector<std::byte>> encode_welcome(const Welcome& welcome) {
    if (welcome.version.major == 0) {
        return std::unexpected(invalid("welcome version cannot be bootstrap"));
    }
    if (welcome.session.is_nil()) {
        return std::unexpected(invalid("welcome session ID is nil"));
    }
    if (welcome.limits.max_control_payload == 0 ||
        welcome.limits.max_control_payload >
            static_cast<std::uint32_t>(kMaxFrameLength - kHeaderBytes)) {
        return std::unexpected(invalid("welcome control payload limit is invalid"));
    }
    if (welcome.limits.max_outstanding_requests == 0 ||
        welcome.limits.max_outstanding_requests > 1024) {
        return std::unexpected(invalid("welcome outstanding-request limit is invalid"));
    }

    Writer writer;
    writer.u8(welcome.version.major);
    writer.u8(welcome.version.minor);
    writer.raw(welcome.session.value().bytes());
    writer.u64(welcome.features);
    writer.u32(welcome.limits.max_control_payload);
    writer.u32(welcome.limits.max_outstanding_requests);
    writer.u8(welcome.resumed ? 1 : 0);
    return std::move(writer).take();
}

Result<Welcome> decode_welcome(std::span<const std::byte> payload) {
    Reader reader(payload);
    std::uint8_t major = 0;
    std::uint8_t minor = 0;
    if (!reader.u8(major) || !reader.u8(minor)) {
        return std::unexpected(invalid("truncated welcome version"));
    }
    if (major == 0) {
        return std::unexpected(invalid("welcome version cannot be bootstrap"));
    }

    std::span<const std::byte> raw_session;
    if (!reader.bytes(kSessionIdBytes, raw_session)) {
        return std::unexpected(invalid("truncated welcome session ID"));
    }
    const SessionId session(session_uuid(raw_session));
    if (session.is_nil()) {
        return std::unexpected(invalid("welcome session ID is nil"));
    }

    Welcome welcome;
    welcome.version = ProtocolVersion{major, minor};
    welcome.session = session;
    if (!reader.u64(welcome.features)) {
        return std::unexpected(invalid("truncated welcome features"));
    }
    if (!reader.u32(welcome.limits.max_control_payload) ||
        welcome.limits.max_control_payload == 0 ||
        welcome.limits.max_control_payload >
            static_cast<std::uint32_t>(kMaxFrameLength - kHeaderBytes)) {
        return std::unexpected(invalid("welcome control payload limit is invalid"));
    }
    if (!reader.u32(welcome.limits.max_outstanding_requests) ||
        welcome.limits.max_outstanding_requests == 0 ||
        welcome.limits.max_outstanding_requests > 1024) {
        return std::unexpected(invalid("welcome outstanding-request limit is invalid"));
    }
    std::uint8_t resumed = 0;
    if (!reader.u8(resumed) || resumed > 1) {
        return std::unexpected(invalid("welcome resumed flag is invalid"));
    }
    welcome.resumed = resumed == 1;

    if (!reader.finished()) {
        return std::unexpected(invalid("trailing bytes in welcome payload"));
    }
    return welcome;
}

Result<std::vector<std::byte>> encode_reject(const Reject& reject) {
    if (!valid_reject_code(reject.code)) {
        return std::unexpected(invalid("reject code is invalid"));
    }
    if (reject.diagnostic.size() > kMaxRejectDiagnosticBytes) {
        return std::unexpected(invalid("reject diagnostic is too long"));
    }

    Writer writer;
    writer.u16(static_cast<std::uint16_t>(reject.code));
    writer.u32(static_cast<std::uint32_t>(reject.diagnostic.size()));
    writer.text(reject.diagnostic);
    return std::move(writer).take();
}

Result<Reject> decode_reject(std::span<const std::byte> payload) {
    Reader reader(payload);
    std::uint16_t code = 0;
    if (!reader.u16(code)) {
        return std::unexpected(invalid("truncated reject code"));
    }
    const auto reject_code = static_cast<HandshakeRejectCode>(code);
    if (!valid_reject_code(reject_code)) {
        return std::unexpected(invalid("reject code is invalid"));
    }
    std::uint32_t diagnostic_length = 0;
    if (!reader.u32(diagnostic_length) ||
        diagnostic_length > kMaxRejectDiagnosticBytes) {
        return std::unexpected(invalid("reject diagnostic length is invalid"));
    }
    Reject reject;
    reject.code = reject_code;
    if (!reader.text(diagnostic_length, reject.diagnostic)) {
        return std::unexpected(invalid("truncated reject diagnostic"));
    }
    if (!reader.finished()) {
        return std::unexpected(invalid("trailing bytes in reject payload"));
    }
    return reject;
}

} // namespace pluribus::fabric
