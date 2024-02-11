#include "tick_manager.hpp"

#include "exchange/logging.hpp"

namespace nutc {
namespace ticks {

auto
TickManager::notify_tick_() // NOLINT
{
    auto start = std::chrono::high_resolution_clock::now();
    for (TickObserver* observer : first_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : second_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : third_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : fourth_observers_) {
        observer->on_tick(current_tick_);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return end - start;
}

void
TickManager::run_()
{
    using namespace std::chrono;
    auto next_tick = steady_clock::now();

    uint64_t tick_delay_mod = static_cast<uint64_t>(milliseconds(1000) / delay_time_);

    while (running_) {
        next_tick += delay_time_;
        std::this_thread::sleep_until(next_tick);
        current_tick_++;
        auto time = notify_tick_();
        if (current_tick_ % tick_delay_mod == 0L) {
            log_i(
                tick_manager, "Tick: {}, Time (ms): {}", current_tick_, duration_cast<milliseconds>(time).count()
            );
        }
    }
}

} // namespace ticks
} // namespace nutc
