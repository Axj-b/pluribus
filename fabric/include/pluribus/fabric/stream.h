#pragma once

#include <cstddef>
#include <memory>
#include <span>

#include "pluribus/core/error.h"

namespace pluribus::fabric {

// Blocking byte-stream boundary. The transport is deliberately narrow so the
// networking stack can change without touching framing, sessions or services.
class Stream {
public:
    virtual ~Stream() = default;

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    // Reads up to buffer.size() bytes. Returns 0 on a clean peer EOF and an
    // error on transport failure. Reads may return fewer bytes than requested.
    virtual Result<std::size_t> read(std::span<std::byte> buffer) = 0;

    // Writes all bytes or returns an error. A failed write may have written a
    // prefix; the caller must close the connection.
    virtual Result<void> write(std::span<const std::byte> bytes) = 0;

    // Idempotent. Closing unblocks local operations where the transport allows.
    virtual Result<void> close() = 0;

protected:
    Stream() = default;
};

struct StreamPair {
    std::unique_ptr<Stream> first;
    std::unique_ptr<Stream> second;
};

// Deterministic in-memory transport for component tests and fake sessions.
StreamPair make_loopback_streams();

} // namespace pluribus::fabric
