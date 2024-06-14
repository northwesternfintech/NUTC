#include "tick_scheduler.hpp"

#include <chrono>

#include <thread>

namespace nutc {
namespace ticks {
using std::chrono::milliseconds;
using steady_clock = std::chrono::steady_clock;

void
run(std::function<void(uint64_t)> function, uint16_t tick_hz)
{
    static constexpr uint16_t MS_PER_SECOND = 1000;

    const auto delay_time = milliseconds(MS_PER_SECOND / tick_hz);
    auto next_tick = steady_clock::now() + delay_time;

    uint64_t current_tick = 0;

    while (true) {
        std::this_thread::sleep_until(next_tick);
        function(current_tick++);
        next_tick = steady_clock::now() + delay_time;
    }
}

} // namespace ticks
} // namespace nutc
