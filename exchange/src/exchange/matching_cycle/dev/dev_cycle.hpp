#pragma once

#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/metrics/on_tick_metrics.hpp"
#include "exchange/traders/trader_container.hpp"

#include <prometheus/counter.h>
#include <prometheus/gauge.h>

namespace nutc {
namespace matching {

class DevMatchingCycle : public BaseMatchingCycle {
    metrics::TickerMetricsPusher pusher;

public:
    DevMatchingCycle(
        TickerMapping tickers, traders::TraderContainer& traders, uint64_t expire_ticks
    ) : BaseMatchingCycle(std::move(tickers), traders, expire_ticks), pusher(traders)
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
        pusher.report_trader_stats(get_tickers());
        pusher.report_ticker_stats(get_tickers());
        BaseMatchingCycle::post_cycle_(new_tick);
    }
};

} // namespace matching
} // namespace nutc
