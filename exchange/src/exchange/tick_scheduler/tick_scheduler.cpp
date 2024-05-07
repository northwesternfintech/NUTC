#include "tick_scheduler.hpp"

#include <numeric>

namespace nutc {
namespace ticks {

auto
TickJobScheduler::notify_tick_()
{
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& job : on_tick_jobs_) {
        job.job_ptr->on_tick(current_tick_);
    }

    auto end = std::chrono::high_resolution_clock::now();
    return end - start;
}

TickJobScheduler::tick_metrics_t
TickJobScheduler::get_tick_metrics() const
{
    std::vector<milliseconds> tmp_ticks{};
    std::ranges::copy(tick_times_, std::back_inserter(tmp_ticks));
    std::ranges::sort(tmp_ticks, std::greater<>());

    auto avg_top_ms = [&](auto num) {
        auto time = std::accumulate(
            tmp_ticks.begin(), std::next(tmp_ticks.begin(), static_cast<int>(num)),
            std::chrono::milliseconds(0)
        );
        return num > 0 ? time / num : std::chrono::milliseconds(0);
    };

    int num_ticks = static_cast<int>(tick_times_.size());
    return {
        avg_top_ms(num_ticks / 100), avg_top_ms(num_ticks / 20),
        avg_top_ms(num_ticks / 10), avg_top_ms(num_ticks / 2), avg_top_ms(num_ticks)
    };
}

void
TickJobScheduler::start(uint16_t tick_hz)
{
    delay_time_ = milliseconds(MS_PER_SECOND / tick_hz);
    using steady_clock = std::chrono::steady_clock;
    auto next_tick = steady_clock::now() + delay_time_;

    while (true) {
        std::this_thread::sleep_until(next_tick);
        current_tick_++;
        auto time = notify_tick_();
        next_tick = steady_clock::now() + delay_time_;

        tick_times_.push_front(duration_cast<milliseconds>(time));
        if (tick_times_.size() > LOOKBACK_TICKS)
            tick_times_.pop_back();
    }
}

} // namespace ticks
} // namespace nutc
