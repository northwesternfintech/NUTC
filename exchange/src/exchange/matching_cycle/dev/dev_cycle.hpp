#pragma once

#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/metrics/on_tick_metrics.hpp"
#include "exchange/traders/trader_container.hpp"

#include <prometheus/counter.h>
#include <prometheus/gauge.h>

namespace nutc::exchange {

class DevMatchingCycle : public BaseMatchingCycle {
    TickerMetricsPusher pusher;

public:
    DevMatchingCycle(TickerContainer tickers, TraderContainer& traders, common::decimal_price order_fee) :
        BaseMatchingCycle(std::move(tickers), traders, order_fee), pusher(traders)
    {}

protected:
    std::vector<common::match>
    match_orders_(std::vector<OrderVariant> orders) override
    {
        // TODO: add back
        pusher.report_orders(orders);
        return BaseMatchingCycle::match_orders_(std::move(orders));
    }

    void
    handle_matches_(std::vector<common::match> matches) override
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

} // namespace nutc::exchange
