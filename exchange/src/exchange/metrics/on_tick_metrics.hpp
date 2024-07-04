#pragma once

#include "exchange/orders/storage/ticker_info.hpp"
#include "shared/ticker.hpp"

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
    ps::Family<ps::Gauge>& holdings_gauge;

    TickerMetricsPusher(std::shared_ptr<ps::Registry> reg);

public:
    TickerMetricsPusher();

    void record_current_tick(uint64_t tick_num);
    void record_trader_metrics(
        const std::unordered_map<util::Ticker, matching::ticker_info>& tickers
    );

    TickerMetricsPusher& operator=(const TickerMetricsPusher&) = delete;
    TickerMetricsPusher(const TickerMetricsPusher&) = delete;
    TickerMetricsPusher& operator=(TickerMetricsPusher&&) = delete;
    TickerMetricsPusher(TickerMetricsPusher&&) = delete;

    void push(
        const std::unordered_map<util::Ticker, matching::ticker_info>& tickers,
        uint64_t tick_num
    );
};
} // namespace metrics
} // namespace nutc
