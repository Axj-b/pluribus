#include "pluribus/fabric/stream.h"

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <utility>

namespace pluribus::fabric {
namespace {

struct LoopbackState {
    std::mutex mutex;
    std::condition_variable changed;
    std::deque<std::byte> first_to_second;
    std::deque<std::byte> second_to_first;
    bool first_open = true;
    bool second_open = true;
};

class LoopbackStream final : public Stream {
public:
    LoopbackStream(std::shared_ptr<LoopbackState> state, bool is_first)
        : state_(std::move(state)), is_first_(is_first) {}

    Result<std::size_t> read(std::span<std::byte> buffer) override {
        std::unique_lock lock(state_->mutex);
        state_->changed.wait(lock, [&] {
            return !read_queue().empty() || !peer_open() || !is_open();
        });

        if (!is_open()) {
            return std::unexpected(
                make_error(ErrorCode::unavailable, "loopback stream is closed"));
        }
        if (read_queue().empty()) {
            return std::size_t{0};
        }
        if (buffer.empty()) {
            return std::size_t{0};
        }

        const std::size_t count = std::min(buffer.size(), read_queue().size());
        for (std::size_t index = 0; index < count; ++index) {
            buffer[index] = read_queue().front();
            read_queue().pop_front();
        }
        state_->changed.notify_all();
        return count;
    }

    Result<void> write(std::span<const std::byte> bytes) override {
        if (bytes.empty()) {
            return {};
        }

        std::unique_lock lock(state_->mutex);
        if (!is_open()) {
            return std::unexpected(
                make_error(ErrorCode::unavailable, "loopback stream is closed"));
        }
        if (!peer_open()) {
            return std::unexpected(
                make_error(ErrorCode::unavailable, "loopback peer is closed"));
        }

        write_queue().insert(write_queue().end(), bytes.begin(), bytes.end());
        state_->changed.notify_all();
        return {};
    }

    Result<void> close() override {
        std::unique_lock lock(state_->mutex);
        if (!is_open()) {
            return {};
        }
        open_flag() = false;
        state_->changed.notify_all();
        return {};
    }

private:
    std::deque<std::byte>& read_queue() {
        return is_first_ ? state_->second_to_first : state_->first_to_second;
    }

    std::deque<std::byte>& write_queue() {
        return is_first_ ? state_->first_to_second : state_->second_to_first;
    }

    bool is_open() const {
        return is_first_ ? state_->first_open : state_->second_open;
    }

    bool& open_flag() {
        return is_first_ ? state_->first_open : state_->second_open;
    }

    bool peer_open() const {
        return is_first_ ? state_->second_open : state_->first_open;
    }

    std::shared_ptr<LoopbackState> state_;
    bool is_first_ = true;
};

} // namespace

StreamPair make_loopback_streams() {
    // Avoid std::make_shared on MinGW/GCC 13: it emits a duplicate
    // std::type_info::operator== that conflicts with static libstdc++.
    auto state = std::shared_ptr<LoopbackState>(new LoopbackState());
    StreamPair pair;
    pair.first = std::make_unique<LoopbackStream>(state, true);
    pair.second = std::make_unique<LoopbackStream>(std::move(state), false);
    return pair;
}

} // namespace pluribus::fabric
