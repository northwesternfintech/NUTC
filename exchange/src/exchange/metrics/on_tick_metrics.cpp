#include "on_tick_metrics.hpp"

#include "exchange/metrics/prometheus.hpp"
#include "exchange/traders/trader_container.hpp"
#include "prometheus.hpp"

#include <prometheus/gauge.h>

namespace nutc {
namespace metrics {

void
TickerMetricsPusher::push(
    std::unordered_map<std::string, matching::ticker_info> tickers, uint64_t tick_num
)
{
    record_current_tick(tick_num);
    record_trader_metrics(tickers);
}

TickerMetricsPusher::TickerMetricsPusher(std::shared_ptr<ps::Registry> reg) :
    pnl_gauge(ps::BuildGauge().Name("pnl").Register(*reg)),
    capital_gauge(ps::BuildGauge().Name("capital").Register(*reg)),
    portfolio_gauge(ps::BuildGauge().Name("portfolio_value").Register(*reg)),
    current_tick(ps::BuildGauge().Name("current_tick").Register(*reg))
{}

TickerMetricsPusher::TickerMetricsPusher() :
    TickerMetricsPusher(metrics::Prometheus::get_registry())
{}

void
TickerMetricsPusher::record_current_tick(uint64_t tick_num)
{
    current_tick.Add({}).Set(static_cast<double>(tick_num));
}

void
TickerMetricsPusher::record_trader_metrics(
    std::unordered_map<std::string, matching::ticker_info> tickers
)
{
    auto& trader_container = traders::TraderContainer::get_instance();

    auto portfolio_value = [&](const auto& trader) {
        double pnl = 0.0;
        for (const auto& [ticker, info] : tickers) {
            double amount_held = trader->get_holdings(ticker);
            double midprice = info.orderbook.get_midprice();
            pnl += amount_held * midprice;
        }
        return pnl;
    };

    for (const auto& trader : trader_container.get_traders()) {
        double capital = trader->get_capital();
        double portfolio = portfolio_value(trader);
        double pnl = capital + portfolio - trader->get_initial_capital();

        pnl_gauge
            .Add({
                {"name",        trader->get_id()  },
                {"trader_type", trader->get_type()}
        })
            .Set(pnl);
        capital_gauge
            .Add({
                {"name",        trader->get_id()  },
                {"trader_type", trader->get_type()}
        })
            .Set(capital);
        portfolio_gauge
            .Add({
                {"name",        trader->get_id()  },
                {"trader_type", trader->get_type()}
        })
            .Set(portfolio);
    }
}

} // namespace metrics
} // namespace nutc
