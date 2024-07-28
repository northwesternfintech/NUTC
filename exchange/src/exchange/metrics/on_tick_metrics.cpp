#include "on_tick_metrics.hpp"

#include "exchange/metrics/prometheus.hpp"
#include "exchange/orders/ticker_info.hpp"
#include "exchange/traders/trader_container.hpp"
#include "prometheus.hpp"

#include <algorithm>

namespace nutc {
namespace metrics {

Gauge
TickerMetricsPusher::create_gauge_(const std::string& gauge_name)
{
    return ps::BuildGauge()
        .Name(gauge_name)
        .Register(*metrics::Prometheus::get_registry());
}

Counter
TickerMetricsPusher::create_counter_(const std::string& gauge_name)
{
    return ps::BuildCounter()
        .Name(gauge_name)
        .Register(*metrics::Prometheus::get_registry());
}

void
TickerMetricsPusher::report_orders(const std::vector<matching::stored_order>& orders)
{
    auto log_order = [&](const matching::stored_order& order) {
        orders_quantity_counter
            .Add({
                {"ticker",      order.position.ticker   },
                {"trader_type", order.trader->get_type()}
        })
            .Increment(order.position.quantity);
    };

    std::for_each(orders.begin(), orders.end(), log_order);
}

void
TickerMetricsPusher::report_ticker_stats(matching::TickerMapping& tickers)
{
    auto log_midprice = [&](util::Ticker ticker, const matching::ticker_info& info) {
        ticker_midprice_gauge
            .Add({
                {"ticker", std::string{ticker}}
        })
            .Set(info.orderbook.get_midprice());
    };
    auto log_best_ba = [&](util::Ticker ticker, matching::ticker_info& info) {
        auto best_bid = info.orderbook.get_top_order(util::Side::buy);
        auto best_ask = info.orderbook.get_top_order(util::Side::sell);

        if (best_bid.has_value()) [[likely]] {
            best_ba_gauge
                .Add({
                    {"ticker", std::string{ticker}},
                    {"type",   "BID"              }
            })
                .Set(best_bid->get().position.price);
        }

        if (best_ask.has_value()) [[likely]] {
            best_ba_gauge
                .Add({
                    {"ticker", std::string{ticker}},
                    {"type",   "ASK"              }
            })
                .Set(best_ask->get().position.price);
        }
    };

    // auto log_variance = [&](util::Ticker ticker, const matching::ticker_info& info) {
    //     ticker_midprice_variance_gauge
    //         .Add({
    //             {"ticker", std::string{ticker}}
    //     })
    //         .Set(info.bot_container.get_variance());
    // };

    for (auto& [info, _, ticker] : tickers) {
        log_midprice(ticker, info);
        log_best_ba(ticker, info);
        // log_variance(ticker, info);
    }
}

void
TickerMetricsPusher::report_matches(const std::vector<matching::stored_match>& orders)
{
    auto log_match = [this](const matching::stored_match& match) {
        matches_quantity_counter
            .Add({
                {"ticker",             match.position.ticker  },
                {"seller_trader_type", match.seller.get_type()},
                {"buyer_trader_type",  match.buyer.get_type() }
        })
            .Increment(match.position.quantity);
    };

    std::for_each(orders.begin(), orders.end(), log_match);
}

void
TickerMetricsPusher::report_current_tick(uint64_t tick_num)
{
    current_tick_gauge.Add({}).Set(static_cast<double>(tick_num));
}

void
TickerMetricsPusher::report_trader_stats(const matching::TickerMapping& tickers)
{
    auto& trader_container = traders::TraderContainer::get_instance();

    auto portfolio_value = [&](const auto& trader) {
        double pnl = 0.0;
        for (const auto& [info, _, ticker] : tickers) {
            double amount_held = trader->get_holdings(ticker);
            double midprice = info.orderbook.get_midprice();
            pnl += amount_held * midprice;
        }
        return pnl;
    };

    auto report_holdings = [&](const auto& trader) {
        for (const auto& [info, _, ticker] : tickers) {
            double amount_held = trader->get_holdings(ticker);
            per_trader_holdings_gauge
                .Add({
                    {"ticker",      ticker            },
                    {"trader_type", trader->get_type()},
                    {"id",          trader->get_id()  },
            })
                .Set(amount_held);
        }
    };

    for (const auto& trader : trader_container.get_traders()) {
        report_holdings(trader);

        double capital = trader->get_capital();
        double portfolio = portfolio_value(trader);
        double pnl = capital + portfolio - trader->get_initial_capital();

        per_trader_pnl_gauge
            .Add({
                {"trader_type", trader->get_type()},
                {"id",          trader->get_id()  },
        })
            .Set(pnl);
        per_trader_capital_gauge
            .Add({
                {"trader_type", trader->get_type()},
                {"id",          trader->get_id()  },
        })
            .Set(capital);
    }
}

} // namespace metrics
} // namespace nutc
