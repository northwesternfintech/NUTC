#pragma once

#include "exchange/tick_scheduler/tick_observer.hpp"

namespace nutc {
namespace metrics {

class OnTickMetricsPush : public ticks::TickObserver {
    OnTickMetricsPush() = default;

public:
    static OnTickMetricsPush&
    get()
    {
        static OnTickMetricsPush instance{};
        return instance;
    }

    OnTickMetricsPush& operator=(const OnTickMetricsPush&) = delete;
    OnTickMetricsPush(const OnTickMetricsPush&) = delete;
    OnTickMetricsPush& operator=(OnTickMetricsPush&&) = delete;
    OnTickMetricsPush(OnTickMetricsPush&&) = delete;

    void on_tick(uint64_t tick_num) override;
};
} // namespace metrics
} // namespace nutc
