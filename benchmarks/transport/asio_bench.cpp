#include <pluribus/fabric/frame.h>

#include <asio.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/use_awaitable.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace pluribus;
using namespace pluribus::fabric;
using Clock = std::chrono::steady_clock;
using tcp = asio::ip::tcp;

std::uint32_t read_be32(std::span<const std::byte> bytes) {
    return (std::to_integer<std::uint32_t>(bytes[0]) << 24) |
           (std::to_integer<std::uint32_t>(bytes[1]) << 16) |
           (std::to_integer<std::uint32_t>(bytes[2]) << 8) |
           std::to_integer<std::uint32_t>(bytes[3]);
}

std::vector<std::byte> make_frame(std::size_t payload_size) {
    Frame frame;
    frame.header.version = kProtocolVersion;
    frame.header.type = 1;
    frame.payload.assign(payload_size, std::byte{0x5A});
    auto encoded = encode_frame(frame);
    if (!encoded) {
        throw std::runtime_error("frame encoding failed");
    }
    return *encoded;
}

asio::awaitable<void> echo_session(tcp::socket socket) {
    try {
        std::vector<std::byte> body;
        for (;;) {
            std::array<std::byte, 4> prefix{};
            co_await asio::async_read(
                socket, asio::buffer(prefix), asio::use_awaitable);
            const auto length = read_be32(prefix);
            if (length < kHeaderBytes || length > kMaxFrameLength) {
                co_return;
            }

            body.resize(length);
            co_await asio::async_read(
                socket, asio::buffer(body), asio::use_awaitable);

            std::vector<asio::const_buffer> buffers{
                asio::buffer(prefix), asio::buffer(body)};
            co_await asio::async_write(
                socket, buffers, asio::use_awaitable);
        }
    } catch (const std::exception&) {
        // Peer closed or the benchmark stopped the context.
    }
}

asio::awaitable<void> accept_loop(tcp::acceptor& acceptor) {
    try {
        for (;;) {
            tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
            asio::co_spawn(
                acceptor.get_executor(),
                echo_session(std::move(socket)),
                asio::detached);
        }
    } catch (const std::exception&) {
        // Listener closed by the benchmark.
    }
}

asio::awaitable<void> client_loop(
    tcp::socket socket,
    std::size_t payload_size,
    int warmup,
    int iterations,
    bool measure_rtt,
    std::vector<double>& samples,
    double& elapsed_seconds,
    std::string& error) {
    try {
        const auto frame = make_frame(payload_size);
        std::vector<std::byte> response(frame.size());

        for (int index = 0; index < warmup; ++index) {
            co_await asio::async_write(
                socket, asio::buffer(frame), asio::use_awaitable);
            co_await asio::async_read(
                socket, asio::buffer(response), asio::use_awaitable);
        }

        const auto start = Clock::now();
        for (int index = 0; index < iterations; ++index) {
            const auto request_start = Clock::now();
            co_await asio::async_write(
                socket, asio::buffer(frame), asio::use_awaitable);
            co_await asio::async_read(
                socket, asio::buffer(response), asio::use_awaitable);
            const auto request_end = Clock::now();

            const auto decoded = decode_frame(response);
            if (!decoded) {
                throw std::runtime_error(decoded.error().diagnostic);
            }
            if (measure_rtt) {
                samples.push_back(
                    std::chrono::duration<double, std::micro>(
                        request_end - request_start)
                        .count());
            }
        }
        const auto end = Clock::now();
        elapsed_seconds = std::chrono::duration<double>(end - start).count();
    } catch (const std::exception& exception) {
        error = exception.what();
    }
}

double percentile(std::vector<double> samples, double fraction) {
    if (samples.empty()) {
        return 0.0;
    }
    std::sort(samples.begin(), samples.end());
    const auto index = static_cast<std::size_t>(
        fraction * static_cast<double>(samples.size() - 1));
    return samples[index];
}

struct CaseResult {
    std::vector<double> samples;
    double elapsed_seconds = 0.0;
    std::string error;
};

CaseResult run_case(
    std::size_t payload_size,
    int warmup,
    int iterations,
    bool measure_rtt) {
    CaseResult result;

    asio::io_context server_context;
    tcp::acceptor acceptor(
        server_context,
        tcp::endpoint(asio::ip::make_address("127.0.0.1"), 0));
    const auto port = acceptor.local_endpoint().port();

    std::thread server([&] {
        asio::co_spawn(
            server_context, accept_loop(acceptor), asio::detached);
        server_context.run();
    });

    asio::io_context client_context;
    tcp::socket socket(client_context);
    socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), port));

    asio::co_spawn(
        client_context,
        client_loop(
            std::move(socket),
            payload_size,
            warmup,
            iterations,
            measure_rtt,
            result.samples,
            result.elapsed_seconds,
            result.error),
        [&](std::exception_ptr exception) {
            if (exception) {
                try {
                    std::rethrow_exception(exception);
                } catch (const std::exception& error) {
                    result.error = error.what();
                }
            }
            client_context.stop();
        });
    client_context.run();

    acceptor.close();
    server_context.stop();
    server.join();
    return result;
}

void run_rtt() {
    constexpr std::size_t kPayloadBytes = 256;
    constexpr int kWarmup = 100;
    constexpr int kIterations = 1000;

    auto result = run_case(kPayloadBytes, kWarmup, kIterations, true);
    if (!result.error.empty()) {
        std::cerr << "asio RTT failed: " << result.error << '\n';
        return;
    }
    std::cout << "asio_tcp_small_frame_rtt_us"
              << " p50=" << percentile(result.samples, 0.50)
              << " p95=" << percentile(result.samples, 0.95)
              << " n=" << result.samples.size()
              << " payload=" << kPayloadBytes << '\n';
}

void run_throughput() {
    constexpr std::size_t kPayloadBytes =
        static_cast<std::size_t>(kMaxFrameLength) - kHeaderBytes;
    constexpr int kIterations = 64;

    auto result = run_case(kPayloadBytes, 0, kIterations, false);
    if (!result.error.empty()) {
        std::cerr << "asio throughput failed: " << result.error << '\n';
        return;
    }

    const double bytes = static_cast<double>(kPayloadBytes) * kIterations;
    std::cout << "asio_tcp_stream_throughput_mib_s="
              << (bytes / (1024.0 * 1024.0)) / result.elapsed_seconds
              << " payload=" << kPayloadBytes
              << " frames=" << kIterations << '\n';
}

} // namespace

int main() {
    try {
        run_rtt();
        run_throughput();
    } catch (const std::exception& exception) {
        std::cerr << "benchmark failed: " << exception.what() << '\n';
        return 1;
    }
    return 0;
}
