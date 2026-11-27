#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "pluribus/fabric/stream.h"

namespace pluribus::fabric {

// Blocking TCP listener. The first transport slice supports IPv4 explicitly
// bound addresses; the caller must not rely on an implicit wildcard bind.
class TcpListener {
public:
    virtual ~TcpListener() = default;

    TcpListener(const TcpListener&) = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    // Binds and listens on address:port. Port zero selects an ephemeral port.
    static Result<std::unique_ptr<TcpListener>> bind(
        std::string_view address,
        std::uint16_t port);

    // Blocks until a peer connects or the listener is closed.
    virtual Result<std::unique_ptr<Stream>> accept() = 0;

    // The bound port, including the selected ephemeral port.
    virtual std::uint16_t port() const noexcept = 0;

    virtual Result<void> close() = 0;

protected:
    TcpListener() = default;
};

// Connects to an IPv4 address and returns a blocking byte stream.
Result<std::unique_ptr<Stream>> connect_tcp(
    std::string_view address,
    std::uint16_t port);

} // namespace pluribus::fabric
