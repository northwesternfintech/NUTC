#include "on_tick_metrics.hpp"

#include "exchange/metrics/prometheus.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_container.hpp"
#include "prometheus.hpp"
#include "shared/config/config_loader.hpp"

#include <prometheus/gauge.h>

namespace nutc {
namespace metrics {

void
OnTickMetricsPush::on_tick(uint64_t tick_num)
{
    record_current_tick(tick_num);
    record_midprices();
    record_trader_metrics();
}

OnTickMetricsPush::OnTickMetricsPush(std::shared_ptr<ps::Registry> reg) :
    pnl_gauge(ps::BuildGauge().Name("pnl").Register(*reg)),
    capital_gauge(ps::BuildGauge().Name("capital").Register(*reg)),
    portfolio_gauge(ps::BuildGauge().Name("portfolio_value").Register(*reg)),
    midprice_gauge(ps::BuildGauge().Name("ticker_midprice").Register(*reg)),
    current_tick(ps::BuildGauge().Name("current_tick").Register(*reg))
{}

OnTickMetricsPush&
OnTickMetricsPush::get()
{
    static OnTickMetricsPush instance{metrics::Prometheus::get_registry()};
    return instance;
}

void
OnTickMetricsPush::record_current_tick(uint64_t tick_num)
{
    current_tick.Add({}).Set(static_cast<double>(tick_num));
}

void
OnTickMetricsPush::record_midprices()
{
    const auto& tickers = config::Config::get().get_tickers();
    const auto& engine_manager = engine_manager::EngineManager::get_instance();

    for (const auto& ticker : tickers) {
        midprice_gauge
            .Add({
                {"ticker", ticker.TICKER}
        })
            .Set(engine_manager.get_midprice(ticker.TICKER));
    }
}

void
OnTickMetricsPush::record_trader_metrics()
{
    const auto& tickers = config::Config::get().get_tickers();
    const auto& engine_manager = engine_manager::EngineManager::get_instance();
    auto& trader_container = traders::TraderContainer::get_instance();

    auto portfolio_value = [&](const auto& trader) {
        double pnl = 0.0;
        for (const auto& ticker_conf : tickers) {
            double amount_held = trader->get_holdings(ticker_conf.TICKER);
            double midprice = engine_manager.get_midprice(ticker_conf.TICKER);
            pnl += amount_held * midprice;
        }
        return pnl;
    };

    for (const auto& trader : trader_container.get_traders()) {
        if (!trader->record_metrics())
            continue;

        double capital = trader->get_capital();
        double portfolio = portfolio_value(trader);
        double pnl = capital + portfolio - trader->get_initial_capital();

        pnl_gauge
            .Add({
                {"name", trader->get_display_name()}
        })
            .Set(pnl);
        capital_gauge
            .Add({
                {"name", trader->get_display_name()}
        })
            .Set(capital);
        portfolio_gauge
            .Add({
                {"name", trader->get_display_name()}
        })
            .Set(portfolio);
    }
}

} // namespace metrics
} // namespace nutc
