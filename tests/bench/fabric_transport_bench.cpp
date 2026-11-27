#include <pluribus/fabric/frame.h>
#include <pluribus/fabric/frame_stream.h>
#include <pluribus/fabric/stream.h>
#include <pluribus/fabric/tcp.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace pluribus;
using namespace pluribus::fabric;
using Clock = std::chrono::steady_clock;

Frame make_frame(std::size_t payload_size, std::uint64_t sequence = 0) {
    Frame frame;
    frame.header.version = kProtocolVersion;
    frame.header.type = 1;
    frame.header.sequence = sequence;
    frame.payload.assign(payload_size, std::byte{0x5A});
    return frame;
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

void run_tcp_rtt() {
    constexpr std::size_t kPayloadBytes = 256;
    constexpr int kWarmup = 100;
    constexpr int kIterations = 1000;

    auto listener = TcpListener::bind("127.0.0.1", 0);
    if (!listener) {
        std::cerr << "bind failed: " << listener.error().diagnostic << '\n';
        return;
    }
    auto client = connect_tcp("127.0.0.1", (*listener)->port());
    if (!client) {
        std::cerr << "connect failed: " << client.error().diagnostic << '\n';
        return;
    }

    std::string server_error;
    std::thread server([&] {
        try {
            auto accepted = (*listener)->accept();
            if (!accepted) {
                server_error = accepted.error().diagnostic;
                return;
            }
            FrameStream stream(std::move(*accepted));
            for (int index = 0; index < kWarmup + kIterations; ++index) {
                const auto frame = stream.read_frame();
                if (!frame || !frame->has_value()) {
                    server_error = "server did not receive a frame";
                    return;
                }
                const auto written = stream.write_frame(**frame);
                if (!written) {
                    server_error = written.error().diagnostic;
                    return;
                }
            }
        } catch (const std::exception& exception) {
            server_error = exception.what();
        }
    });

    FrameStream stream(std::move(*client));
    for (int index = 0; index < kWarmup; ++index) {
        const Frame frame = make_frame(kPayloadBytes, static_cast<std::uint64_t>(index));
        const auto written = stream.write_frame(frame);
        const auto response = stream.read_frame();
        if (!written || !response || !response->has_value()) {
            std::cerr << "TCP RTT warmup failed\n";
            server.join();
            return;
        }
    }

    std::vector<double> samples;
    samples.reserve(kIterations);
    for (int index = 0; index < kIterations; ++index) {
        const Frame frame = make_frame(kPayloadBytes, static_cast<std::uint64_t>(index));
        const auto start = Clock::now();
        if (!stream.write_frame(frame)) {
            std::cerr << "TCP RTT write failed\n";
            break;
        }
        const auto response = stream.read_frame();
        const auto end = Clock::now();
        if (!response || !response->has_value()) {
            std::cerr << "TCP RTT read failed\n";
            break;
        }
        samples.push_back(std::chrono::duration<double, std::micro>(end - start).count());
    }

    server.join();
    if (!server_error.empty()) {
        std::cerr << "server error: " << server_error << '\n';
    }
    if (!samples.empty()) {
        std::cout << "tcp_small_frame_rtt_us"
                  << " p50=" << percentile(samples, 0.50)
                  << " p95=" << percentile(samples, 0.95)
                  << " n=" << samples.size()
                  << " payload=" << kPayloadBytes << '\n';
    }
    (void)stream.close();
    (void)(*listener)->close();
}

void run_tcp_throughput() {
    constexpr std::size_t kPayloadBytes =
        static_cast<std::size_t>(kMaxFrameLength) - kHeaderBytes;
    constexpr int kIterations = 64;

    auto listener = TcpListener::bind("127.0.0.1", 0);
    if (!listener) {
        std::cerr << "bind failed: " << listener.error().diagnostic << '\n';
        return;
    }
    auto client = connect_tcp("127.0.0.1", (*listener)->port());
    if (!client) {
        std::cerr << "connect failed: " << client.error().diagnostic << '\n';
        return;
    }

    std::string server_error;
    std::thread server([&] {
        try {
            auto accepted = (*listener)->accept();
            if (!accepted) {
                server_error = accepted.error().diagnostic;
                return;
            }
            FrameStream stream(std::move(*accepted));
            for (int index = 0; index < kIterations; ++index) {
                const auto frame = stream.read_frame();
                if (!frame || !frame->has_value()) {
                    server_error = "server did not receive a throughput frame";
                    return;
                }
            }
            const auto ack = stream.write_frame(make_frame(8));
            if (!ack) {
                server_error = ack.error().diagnostic;
            }
        } catch (const std::exception& exception) {
            server_error = exception.what();
        }
    });

    FrameStream stream(std::move(*client));
    const Frame frame = make_frame(kPayloadBytes);
    bool write_ok = true;
    const auto start = Clock::now();
    for (int index = 0; index < kIterations; ++index) {
        if (!stream.write_frame(frame)) {
            std::cerr << "TCP throughput write failed\n";
            write_ok = false;
            (void)stream.close();
            break;
        }
    }
    std::optional<Frame> ack_frame;
    if (write_ok) {
        const auto ack = stream.read_frame();
        if (ack && ack->has_value()) {
            ack_frame = **ack;
        }
    }
    const auto end = Clock::now();
    server.join();

    if (!server_error.empty()) {
        std::cerr << "server error: " << server_error << '\n';
    }
    if (!write_ok || !ack_frame.has_value()) {
        std::cerr << "TCP throughput ack failed\n";
        return;
    }
    const double seconds = std::chrono::duration<double>(end - start).count();
    const double bytes = static_cast<double>(kPayloadBytes) * kIterations;
    std::cout << "tcp_stream_throughput_mib_s=" << (bytes / (1024.0 * 1024.0)) / seconds
              << " payload=" << kPayloadBytes
              << " frames=" << kIterations << '\n';
    (void)stream.close();
    (void)(*listener)->close();
}

void run_memory_throughput() {
    constexpr std::size_t kPayloadBytes =
        static_cast<std::size_t>(kMaxFrameLength) - kHeaderBytes;
    constexpr int kIterations = 64;

    auto pair = make_loopback_streams();
    FrameStream writer(std::move(pair.first));
    FrameStream reader(std::move(pair.second));
    const Frame frame = make_frame(kPayloadBytes);

    const auto start = Clock::now();
    for (int index = 0; index < kIterations; ++index) {
        if (!writer.write_frame(frame)) {
            std::cerr << "memory throughput write failed\n";
            return;
        }
        const auto received = reader.read_frame();
        if (!received || !received->has_value()) {
            std::cerr << "memory throughput read failed\n";
            return;
        }
    }
    const auto end = Clock::now();
    const double seconds = std::chrono::duration<double>(end - start).count();
    const double bytes = static_cast<double>(kPayloadBytes) * kIterations;
    std::cout << "memory_frame_throughput_mib_s=" << (bytes / (1024.0 * 1024.0)) / seconds
              << " payload=" << kPayloadBytes
              << " frames=" << kIterations << '\n';
}

} // namespace

int main() {
    run_tcp_rtt();
    run_tcp_throughput();
    run_memory_throughput();
    return 0;
}
