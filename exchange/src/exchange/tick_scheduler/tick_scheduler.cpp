#include "tick_scheduler.hpp"

namespace nutc {
namespace ticks {

void
TickJobScheduler::notify_tick_()
{
    for (const auto& job : on_tick_jobs_) {
        job.job_ptr->on_tick(current_tick_);
    }
}

void
TickJobScheduler::run(uint16_t tick_hz)
{
    delay_time_ = milliseconds(MS_PER_SECOND / tick_hz);
    using steady_clock = std::chrono::steady_clock;
    auto next_tick = steady_clock::now() + delay_time_;

    while (true) {
        std::this_thread::sleep_until(next_tick);
        current_tick_++;
        notify_tick_();
        next_tick = steady_clock::now() + delay_time_;
    }
}

} // namespace ticks
} // namespace nutc
