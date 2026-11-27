#include "pluribus/fabric/tcp.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <climits>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace pluribus::fabric {
namespace {

Error make_wsa_error(std::string_view action) {
    const int code = WSAGetLastError();
    ErrorCode error_code = ErrorCode::internal;
    switch (code) {
    case WSAETIMEDOUT:
        error_code = ErrorCode::timeout;
        break;
    case WSAECONNREFUSED:
    case WSAENETUNREACH:
    case WSAEHOSTUNREACH:
    case WSAECONNRESET:
    case WSAECONNABORTED:
    case WSAENOTCONN:
        error_code = ErrorCode::unavailable;
        break;
    case WSAEINVAL:
        error_code = ErrorCode::invalid_argument;
        break;
    case WSAEMFILE:
    case WSAENOBUFS:
        error_code = ErrorCode::resource_exhausted;
        break;
    default:
        error_code = ErrorCode::internal;
        break;
    }

    return make_error(
        error_code,
        std::string(action) + " failed with Winsock error " + std::to_string(code));
}

Result<void> ensure_winsock() {
    static const int startup_result = [] {
        WSADATA data{};
        return WSAStartup(MAKEWORD(2, 2), &data);
    }();

    if (startup_result != 0) {
        return std::unexpected(make_error(
            ErrorCode::internal,
            "WSAStartup failed with error " + std::to_string(startup_result)));
    }
    return {};
}

void configure_socket(SOCKET socket) {
    int enabled = 1;
    setsockopt(
        socket,
        IPPROTO_TCP,
        TCP_NODELAY,
        reinterpret_cast<const char*>(&enabled),
        static_cast<int>(sizeof(enabled)));
}

class Win32Stream final : public Stream {
public:
    explicit Win32Stream(SOCKET socket) : socket_(socket) {}
    ~Win32Stream() override { (void)close(); }

    Result<std::size_t> read(std::span<std::byte> buffer) override {
        if (socket_ == INVALID_SOCKET) {
            return std::unexpected(
                make_error(ErrorCode::unavailable, "TCP stream is closed"));
        }
        if (buffer.empty()) {
            return std::size_t{0};
        }

        const int chunk = static_cast<int>(
            std::min(buffer.size(), static_cast<std::size_t>(INT_MAX)));
        const int received = ::recv(
            socket_, reinterpret_cast<char*>(buffer.data()), chunk, 0);
        if (received == 0) {
            return std::size_t{0};
        }
        if (received == SOCKET_ERROR) {
            return std::unexpected(make_wsa_error("recv"));
        }
        return static_cast<std::size_t>(received);
    }

    Result<void> write(std::span<const std::byte> bytes) override {
        if (socket_ == INVALID_SOCKET) {
            return std::unexpected(
                make_error(ErrorCode::unavailable, "TCP stream is closed"));
        }

        std::size_t offset = 0;
        while (offset < bytes.size()) {
            const int chunk = static_cast<int>(std::min(
                bytes.size() - offset, static_cast<std::size_t>(INT_MAX)));
            const int sent = ::send(
                socket_,
                reinterpret_cast<const char*>(bytes.data() + offset),
                chunk,
                0);
            if (sent == SOCKET_ERROR) {
                return std::unexpected(make_wsa_error("send"));
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
        if (socket_ != INVALID_SOCKET) {
            (void)::closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }
        return {};
    }

private:
    SOCKET socket_ = INVALID_SOCKET;
};

class Win32Listener final : public TcpListener {
public:
    Win32Listener(SOCKET socket, std::uint16_t port) : socket_(socket), port_(port) {}
    ~Win32Listener() override { (void)close(); }

    Result<std::unique_ptr<Stream>> accept() override {
        if (socket_ == INVALID_SOCKET) {
            return std::unexpected(
                make_error(ErrorCode::unavailable, "TCP listener is closed"));
        }

        SOCKET client = INVALID_SOCKET;
        do {
            client = ::accept(socket_, nullptr, nullptr);
        } while (client == INVALID_SOCKET && WSAGetLastError() == WSAEINTR);

        if (client == INVALID_SOCKET) {
            return std::unexpected(make_wsa_error("accept"));
        }
        configure_socket(client);
        return std::unique_ptr<Stream>(new Win32Stream(client));
    }

    std::uint16_t port() const noexcept override { return port_; }

    Result<void> close() override {
        if (socket_ != INVALID_SOCKET) {
            (void)::closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }
        return {};
    }

private:
    SOCKET socket_ = INVALID_SOCKET;
    std::uint16_t port_ = 0;
};

} // namespace

Result<std::unique_ptr<TcpListener>> TcpListener::bind(
    std::string_view address,
    std::uint16_t port) {
    if (auto winsock = ensure_winsock(); !winsock) {
        return std::unexpected(winsock.error());
    }

    sockaddr_in endpoint{};
    endpoint.sin_family = AF_INET;
    endpoint.sin_port = htons(port);
    const std::string address_text(address);
    if (::inet_pton(AF_INET, address_text.c_str(), &endpoint.sin_addr) != 1) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument,
            "TCP bind address is not a valid IPv4 address: " + address_text));
    }

    SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET) {
        return std::unexpected(make_wsa_error("socket"));
    }

    int reuse = 1;
    (void)setsockopt(
        socket,
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<const char*>(&reuse),
        static_cast<int>(sizeof(reuse)));

    if (::bind(socket, reinterpret_cast<const sockaddr*>(&endpoint), sizeof(endpoint)) ==
        SOCKET_ERROR) {
        auto error = make_wsa_error("bind");
        (void)::closesocket(socket);
        return std::unexpected(std::move(error));
    }
    if (::listen(socket, SOMAXCONN) == SOCKET_ERROR) {
        auto error = make_wsa_error("listen");
        (void)::closesocket(socket);
        return std::unexpected(std::move(error));
    }

    sockaddr_in bound{};
    int bound_length = static_cast<int>(sizeof(bound));
    if (::getsockname(
            socket, reinterpret_cast<sockaddr*>(&bound), &bound_length) == SOCKET_ERROR) {
        auto error = make_wsa_error("getsockname");
        (void)::closesocket(socket);
        return std::unexpected(std::move(error));
    }

    return std::unique_ptr<TcpListener>(
        new Win32Listener(socket, ntohs(bound.sin_port)));
}

Result<std::unique_ptr<Stream>> connect_tcp(std::string_view address, std::uint16_t port) {
    if (auto winsock = ensure_winsock(); !winsock) {
        return std::unexpected(winsock.error());
    }

    sockaddr_in endpoint{};
    endpoint.sin_family = AF_INET;
    endpoint.sin_port = htons(port);
    const std::string address_text(address);
    if (::inet_pton(AF_INET, address_text.c_str(), &endpoint.sin_addr) != 1) {
        return std::unexpected(make_error(
            ErrorCode::invalid_argument,
            "TCP connect address is not a valid IPv4 address: " + address_text));
    }

    SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET) {
        return std::unexpected(make_wsa_error("socket"));
    }

    if (::connect(
            socket,
            reinterpret_cast<const sockaddr*>(&endpoint),
            sizeof(endpoint)) == SOCKET_ERROR) {
        auto error = make_wsa_error("connect");
        (void)::closesocket(socket);
        return std::unexpected(std::move(error));
    }

    configure_socket(socket);
    return std::unique_ptr<Stream>(new Win32Stream(socket));
}

} // namespace pluribus::fabric
