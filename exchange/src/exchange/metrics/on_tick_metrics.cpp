#include "on_tick_metrics.hpp"

#include "exchange/metrics/prometheus.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/config/config_loader.hpp"

#include <prometheus/gauge.h>

namespace nutc {
namespace metrics {

namespace ps = prometheus;

void
OnTickMetricsPush::on_tick(uint64_t tick_num)

{
    static auto registry = nutc::metrics::Prometheus::get_registry();
    static auto& pnl_gauge = ps::BuildGauge().Name("pnl").Register(*registry);
    static auto& capital_gauge = ps::BuildGauge().Name("capital").Register(*registry);
    static auto& portfolio_gauge =
        ps::BuildGauge().Name("portfolio_value").Register(*registry);
    static auto& midprice_gauge =
        ps::BuildGauge().Name("ticker_midprice").Register(*registry);
    static auto& current_tick =
        ps::BuildGauge().Name("current_tick").Register(*registry).Add({});
    current_tick.Set(static_cast<double>(tick_num));
    const auto& tickers = config::Config::get().get_tickers();
    for (const auto& ticker : tickers) {
        midprice_gauge
            .Add({
                {"ticker", ticker.TICKER}
        })
            .Set(engine_manager::EngineManager::get_instance().get_midprice(
                ticker.TICKER
            ));
    }

    auto portfolio_value = [&](const auto& trader) {
        double pnl = 0.0;
        for (const auto& ticker_conf : tickers) {
            double amount_held = trader->get_holdings(ticker_conf.TICKER);
            double midprice =
                engine_manager::EngineManager::get_instance().get_midprice(
                    ticker_conf.TICKER
                );
            pnl += amount_held * midprice;
        }
        return pnl;
    };

    traders::TraderContainer& client_manager = traders::TraderContainer::get_instance();
    for (const auto& trader_p : client_manager.get_traders()) {
        auto trader = trader_p.second;
        if (!trader->record_metrics())
            continue;

        double capital = trader->get_capital();
        double portfolio = portfolio_value(trader);
        double pnl = capital + portfolio - trader->get_initial_capital();
        auto name = trader->get_id();
        pnl_gauge
            .Add({
                {"name", name}
        })
            .Set(pnl);
        capital_gauge
            .Add({
                {"name", name}
        })
            .Set(capital);
        portfolio_gauge
            .Add({
                {"name", name}
        })
            .Set(portfolio);
    }
}

} // namespace metrics
} // namespace nutc
