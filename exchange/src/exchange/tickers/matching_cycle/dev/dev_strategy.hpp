#pragma once

#include "exchange/metrics/on_tick_metrics.hpp"
#include "exchange/metrics/prometheus.hpp"
#include "exchange/tickers/matching_cycle/base/base_strategy.hpp"
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
    virtual void
    before_cycle_(uint64_t new_tick) override
    {
        for (auto& [symbol, ticker_info] : tickers_) {
            log_midprice_(symbol, ticker_info.orderbook);
        }
        BaseMatchingCycle::before_cycle_(new_tick);
    }

    virtual void
    post_cycle_(uint64_t new_tick) override
    {
        pusher.push(tickers_, new_tick);
        BaseMatchingCycle::post_cycle_(new_tick);
    }

private:
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
