#pragma once

#include "exchange/matching_cycle/base/base_strategy.hpp"
#include "exchange/metrics/on_tick_metrics.hpp"
#include "exchange/metrics/prometheus.hpp"
#include "shared/util.hpp"

#include <prometheus/counter.h>
#include <prometheus/gauge.h>

namespace nutc {
namespace matching {
class DevMatchingCycle : public BaseMatchingCycle {
    metrics::TickerMetricsPusher pusher;

public:
    DevMatchingCycle(
        std::unordered_map<util::Ticker, ticker_info> tickers,
        std::pmr::vector<std::shared_ptr<traders::GenericTrader>> traders,
        uint64_t expire_ticks
    ) : BaseMatchingCycle(tickers, std::move(traders), expire_ticks)
    {}

protected:
    void
    before_cycle_(uint64_t new_tick) override
    {
        for (auto& [symbol, ticker_info] : tickers_) {
            log_midprice_(symbol, ticker_info.orderbook);
        }
        BaseMatchingCycle::before_cycle_(new_tick);
    }

    std::vector<stored_match>
    match_orders_(std::vector<stored_order> orders) override
    {
        for (auto& order : orders) {
            log_order_(order);
        }
        return BaseMatchingCycle::match_orders_(std::move(orders));
    }

    void
    handle_matches_(std::vector<stored_match> matches) override
    {
        for (const auto& match : matches) {
            log_match_(match);
        }
        BaseMatchingCycle::handle_matches_(std::move(matches));
    }

    void
    post_cycle_(uint64_t new_tick) override
    {
        pusher.push(tickers_, new_tick);
        BaseMatchingCycle::post_cycle_(new_tick);
    }

private:
    void
    log_match_(const stored_match& match)
    {
        static auto& match_counter =
            prometheus::BuildCounter()
                .Name("matches_total")
                .Register(*metrics::Prometheus::get_registry());

        match_counter
            .Add({
                {"ticker",             match.ticker           },
                {"seller_trader_type", match.seller.get_type()},
                {"buyer_trader_type",  match.buyer.get_type() }
        })
            .Increment(match.quantity);
    }

    void
    log_order_(const stored_order& order)
    {
        static auto& order_counter =
            prometheus::BuildCounter()
                .Name("orders_total")
                .Register(*metrics::Prometheus::get_registry());

        order_counter
            .Add({
                {"ticker",      order.ticker           },
                {"trader_type", order.trader.get_type()}
        })
            .Increment(order.quantity);
    }

    void
    log_midprice_(util::Ticker symbol, const OrderBook& orderbook)
    {
        static auto& midprice_gauge =
            prometheus::BuildGauge()
                .Name("ticker_midprice")
                .Register(*metrics::Prometheus::get_registry());

        auto midprice = orderbook.get_midprice();
        midprice_gauge
            .Add({
                {"ticker", symbol}
        })
            .Set(midprice);
    }
};

} // namespace matching
} // namespace nutc
