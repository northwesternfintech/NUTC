#pragma once

#include "exchange/tickers/ticker.hpp"

#include <prometheus/family.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>

#include <unordered_map>

namespace nutc {
namespace metrics {
namespace ps = prometheus;

class TickerMetricsPusher {
    ps::Family<ps::Gauge>& pnl_gauge;
    ps::Family<ps::Gauge>& capital_gauge;
    ps::Family<ps::Gauge>& portfolio_gauge;
    ps::Family<ps::Gauge>& current_tick;

    TickerMetricsPusher(std::shared_ptr<ps::Registry> reg);

public:
    TickerMetricsPusher();

    void record_current_tick(uint64_t tick_num);
    void
    record_trader_metrics(std::unordered_map<std::string, matching::ticker_info> tickers
    );

    TickerMetricsPusher& operator=(const TickerMetricsPusher&) = delete;
    TickerMetricsPusher(const TickerMetricsPusher&) = delete;
    TickerMetricsPusher& operator=(TickerMetricsPusher&&) = delete;
    TickerMetricsPusher(TickerMetricsPusher&&) = delete;

    void push(
        std::unordered_map<std::string, matching::ticker_info> tickers,
        uint64_t tick_num
    );
};
} // namespace metrics
} // namespace nutc
