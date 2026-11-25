#include "pluribus/fabric/tcp.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <utility>

namespace pluribus::fabric {
namespace {

Error make_errno_error(std::string_view action) {
    const int code = errno;
    ErrorCode error_code = ErrorCode::internal;
    switch (code) {
    case ETIMEDOUT:
        error_code = ErrorCode::timeout;
        break;
    case ECONNREFUSED:
    case ENETUNREACH:
    case EHOSTUNREACH:
    case ECONNRESET:
    case ECONNABORTED:
    case EPIPE:
    case ENOTCONN:
        error_code = ErrorCode::unavailable;
        break;
    case EINVAL:
        error_code = ErrorCode::invalid_argument;
        break;
    case EMFILE:
    case ENFILE:
    case ENOBUFS:
    case ENOMEM:
        error_code = ErrorCode::resource_exhausted;
        break;
    default:
        error_code = ErrorCode::internal;
        break;
    }

    return make_error(
        error_code,
        std::string(action) + " failed with errno " + std::to_string(code));
}

void configure_socket(int socket) {
    int enabled = 1;
    setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &enabled, sizeof(enabled));
#ifdef SO_NOSIGPIPE
    setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &enabled, sizeof(enabled));
#endif
}

class PosixStream final : public Stream {
public:
    explicit PosixStream(int socket) : socket_(socket) {}
    ~PosixStream() override { (void)close(); }

    Result<std::size_t> read(std::span<std::byte> buffer) override {
        if (socket_ < 0) {
            return std::unexpected(
                make_error(ErrorCode::unavailable, "TCP stream is closed"));
        }
        if (buffer.empty()) {
            return std::size_t{0};
        }

        std::size_t offset = 0;
        while (offset < buffer.size()) {
            const std::size_t chunk = std::min(
                buffer.size() - offset,
                static_cast<std::size_t>(std::numeric_limits<ssize_t>::max()));
            const ssize_t received = ::recv(socket_, buffer.data() + offset, chunk, 0);
            if (received < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return std::unexpected(make_errno_error("recv"));
            }
            if (received == 0) {
                return offset;
            }
            offset += static_cast<std::size_t>(received);
            break;
        }
        return offset;
    }

    Result<void> write(std::span<const std::byte> bytes) override {
        if (socket_ < 0) {
            return std::unexpected(
                make_error(ErrorCode::unavailable, "TCP stream is closed"));
        }

        std::size_t offset = 0;
        while (offset < bytes.size()) {
            const std::size_t chunk = std::min(
                bytes.size() - offset,
                static_cast<std::size_t>(std::numeric_limits<ssize_t>::max()));
            int flags = 0;
#ifdef MSG_NOSIGNAL
            flags = MSG_NOSIGNAL;
#endif
            const ssize_t sent = ::send(socket_, bytes.data() + offset, chunk, flags);
            if (sent < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return std::unexpected(make_errno_error("send"));
            }
            if (sent == 0) {
                return std::unexpected(
                    make_error(ErrorCode::unavailable, "send made no progress"));
            }
            offset += static_cast<std::size_t>(sent);
        }
        return {};
    }

    Result<void> close() override {
        if (socket_ >= 0) {
            (void)::close(socket_);
            socket_ = -1;
        }
        return {};
    }

private:
    int socket_ = -1;
};

class PosixListener final : public TcpListener {
public:
    PosixListener(int socket, std::uint16_t port) : socket_(socket), port_(port) {}
    ~PosixListener() override { (void)close(); }

    Result<std::unique_ptr<Stream>> accept() override {
        if (socket_ < 0) {
            return std::unexpected(
                make_error(ErrorCode::unavailable, "TCP listener is closed"));
        }

        int client = -1;
        do {
            client = ::accept(socket_, nullptr, nullptr);
        } while (client < 0 && errno == EINTR);

        if (client < 0) {
            return std::unexpected(make_errno_error("accept"));
        }
        configure_socket(client);
        return std::unique_ptr<Stream>(new PosixStream(client));
    }

    std::uint16_t port() const noexcept override { return port_; }

    Result<void> close() override {
        if (socket_ >= 0) {
            (void)::close(socket_);
            socket_ = -1;
        }
        return {};
    }

private:
    int socket_ = -1;
    std::uint16_t port_ = 0;
};

} // namespace

Result<std::unique_ptr<TcpListener>> TcpListener::bind(
    std::string_view address,
    std::uint16_t port) {
    sockaddr_in endpoint{};
    endpoint.sin_family = AF_INET;
    endpoint.sin_port = htons(port);
    const std::string address_text(address);
    if (::inet_pton(AF_INET, address_text.c_str(), &endpoint.sin_addr) != 1) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument,
            "TCP bind address is not a valid IPv4 address: " + address_text));
    }

    const int socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket < 0) {
        return std::unexpected(make_errno_error("socket"));
    }

    int reuse = 1;
    (void)setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (::bind(socket, reinterpret_cast<const sockaddr*>(&endpoint), sizeof(endpoint)) < 0) {
        auto error = make_errno_error("bind");
        (void)::close(socket);
        return std::unexpected(std::move(error));
    }
    if (::listen(socket, SOMAXCONN) < 0) {
        auto error = make_errno_error("listen");
        (void)::close(socket);
        return std::unexpected(std::move(error));
    }

    sockaddr_in bound{};
    socklen_t bound_length = sizeof(bound);
    if (::getsockname(socket, reinterpret_cast<sockaddr*>(&bound), &bound_length) < 0) {
        auto error = make_errno_error("getsockname");
        (void)::close(socket);
        return std::unexpected(std::move(error));
    }

    return std::unique_ptr<TcpListener>(
        new PosixListener(socket, ntohs(bound.sin_port)));
}

Result<std::unique_ptr<Stream>> connect_tcp(std::string_view address, std::uint16_t port) {
    sockaddr_in endpoint{};
    endpoint.sin_family = AF_INET;
    endpoint.sin_port = htons(port);
    const std::string address_text(address);
    if (::inet_pton(AF_INET, address_text.c_str(), &endpoint.sin_addr) != 1) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument,
            "TCP connect address is not a valid IPv4 address: " + address_text));
    }

    const int socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket < 0) {
        return std::unexpected(make_errno_error("socket"));
    }

    int result = -1;
    do {
        result = ::connect(
            socket,
            reinterpret_cast<const sockaddr*>(&endpoint),
            sizeof(endpoint));
    } while (result < 0 && errno == EINTR);

    if (result < 0) {
        auto error = make_errno_error("connect");
        (void)::close(socket);
        return std::unexpected(std::move(error));
    }

    configure_socket(socket);
    return std::unique_ptr<Stream>(new PosixStream(socket));
}

} // namespace pluribus::fabric
