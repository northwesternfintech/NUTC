#pragma once

#include "exchange/logging.hpp"
#include "tick_observer.hpp"

#include <cassert>
#include <chrono>

namespace nutc {
namespace ticks {

using std::chrono::milliseconds;

class TickJobScheduler {
    static constexpr uint16_t MS_PER_SECOND = 1000;
    static constexpr uint16_t LOOKBACK_TICKS = 1000;

    struct scheduled_job {
        uint8_t priority;
        std::shared_ptr<TickObserver> job_ptr;
    };

    uint64_t current_tick_{};
    milliseconds delay_time_{};
    std::vector<scheduled_job> on_tick_jobs_{};

public:
    [[nodiscard]] uint64_t
    get_current_tick() const
    {
        return current_tick_;
    }

    void
    on_tick(
        const std::shared_ptr<TickObserver>& observer, uint8_t priority,
        const std::string& name = "UNNAMED"
    )
    {
        // Assert we have not already added this job
        assert(!std::ranges::any_of(on_tick_jobs_, [&](const auto& job) {
            return job.job_ptr == observer;
        }));
        log_i(
            tick_scheduler, "Tick job scheduler registered job {} with priority {}",
            name, priority
        );
        on_tick_jobs_.push_back({priority, observer});
        std::ranges::sort(on_tick_jobs_, {}, &scheduled_job::priority);
    }

    void
    detach(const std::shared_ptr<TickObserver>& observer)
    {
        std::erase_if(on_tick_jobs_, [&](const auto& job) {
            return job.job_ptr == observer;
        });
    }

    void run(uint16_t tick_hz);

private:
    void notify_tick_();

public:
    TickJobScheduler() = default;
    TickJobScheduler(const TickJobScheduler&) = delete;
    TickJobScheduler& operator=(const TickJobScheduler&) = delete;
    TickJobScheduler(TickJobScheduler&&) = default;
    TickJobScheduler& operator=(TickJobScheduler&&) = default;

    ~TickJobScheduler() = default;
};
} // namespace ticks
} // namespace nutc
