#pragma once

#include "exchange/matching_cycle/base/base_strategy.hpp"
#include "exchange/metrics/on_tick_metrics.hpp"

#include <prometheus/counter.h>
#include <prometheus/gauge.h>

namespace nutc {
namespace matching {
class DevMatchingCycle : public BaseMatchingCycle {
    metrics::TickerMetricsPusher pusher;

public:
    DevMatchingCycle(
        emhash7::HashMap<util::Ticker, ticker_info> tickers,
        std::vector<std::shared_ptr<traders::GenericTrader>>& traders,
        uint64_t expire_ticks
    ) : BaseMatchingCycle(std::move(tickers), traders, expire_ticks)
    {}

protected:
    std::vector<stored_match>
    match_orders_(std::vector<stored_order> orders) override
    {
        pusher.report_orders(orders);
        return BaseMatchingCycle::match_orders_(std::move(orders));
    }

    void
    handle_matches_(std::vector<stored_match> matches) override
    {
        pusher.report_matches(matches);
        BaseMatchingCycle::handle_matches_(std::move(matches));
    }

    void
    post_cycle_(uint64_t new_tick) override
    {
        pusher.report_current_tick(new_tick);
        pusher.report_trader_stats(tickers_);
        pusher.report_ticker_stats(tickers_);
        BaseMatchingCycle::post_cycle_(new_tick);
    }
};

} // namespace matching
} // namespace nutc
