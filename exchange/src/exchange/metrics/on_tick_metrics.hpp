#pragma once

#include "exchange/tick_scheduler/tick_observer.hpp"

#include <prometheus/family.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>

namespace nutc {
namespace metrics {
namespace ps = prometheus;

class OnTickMetricsPush : public ticks::TickObserver {
    ps::Family<ps::Gauge>& pnl_gauge;
    ps::Family<ps::Gauge>& capital_gauge;
    ps::Family<ps::Gauge>& portfolio_gauge;
    ps::Family<ps::Gauge>& midprice_gauge;
    ps::Family<ps::Gauge>& current_tick;

    OnTickMetricsPush(std::shared_ptr<ps::Registry> reg);

public:
    static OnTickMetricsPush& get();

    void record_current_tick(uint64_t tick_num);
    void record_midprices();
    void record_trader_metrics();

    OnTickMetricsPush& operator=(const OnTickMetricsPush&) = delete;
    OnTickMetricsPush(const OnTickMetricsPush&) = delete;
    OnTickMetricsPush& operator=(OnTickMetricsPush&&) = delete;
    OnTickMetricsPush(OnTickMetricsPush&&) = delete;

    void on_tick(uint64_t tick_num) override;
};
} // namespace metrics
} // namespace nutc
