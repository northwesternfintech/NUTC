#pragma once

#include "exchange/orders/ticker_container.hpp"
#include "exchange/traders/trader_container.hpp"
#include "common/messages_exchange_to_wrapper.hpp"

#include <hash_table7.hpp>
#include <prometheus/counter.h>
#include <prometheus/family.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>

namespace nutc::exchange {

namespace ps = prometheus;
using Gauge = ps::Family<ps::Gauge>&;
using Counter = ps::Family<ps::Counter>&;

class TickerMetricsPusher {
    TraderContainer& trader_container_;
    Gauge per_trader_pnl_gauge = create_gauge_("pnl");
    Gauge per_trader_capital_gauge = create_gauge_("capital");
    Gauge per_trader_holdings_gauge = create_gauge_("holdings");
    Gauge ticker_midprice_gauge = create_gauge_("ticker_midprice");
    Gauge best_ba_gauge = create_gauge_("best_ba");
    Gauge ticker_midprice_variance_gauge = create_gauge_("ticker_midprice_variance");
    Counter matches_quantity_counter = create_counter_("matches_quantity_total");
    Counter orders_quantity_counter = create_counter_("orders_quantity_total");
    Counter cancellation_counter = create_counter_("order_cancellations_total");
    Gauge current_tick_gauge = create_gauge_("current_tick");

public:
    TickerMetricsPusher(TraderContainer& trader_container) :
        trader_container_(trader_container)
    {}

    void report_current_tick(uint64_t tick_num);
    void report_trader_stats(const TickerContainer& tickers);
    void report_ticker_stats(TickerContainer& tickers);
    void report_orders(const std::vector<OrderVariant>& orders);
    void report_matches(const std::vector<common::match>& orders);

private:
    Gauge create_gauge_(const std::string& gauge_name);
    Counter create_counter_(const std::string& counter_name);
};
} // namespace nutc::exchange
