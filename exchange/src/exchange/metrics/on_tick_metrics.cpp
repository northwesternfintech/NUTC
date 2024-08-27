#include "on_tick_metrics.hpp"

#include "exchange/metrics/prometheus.hpp"
#include "exchange/orders/ticker_info.hpp"
#include "exchange/traders/trader_container.hpp"
#include "prometheus.hpp"

#include <algorithm>

namespace nutc::exchange {

Gauge
TickerMetricsPusher::create_gauge_(const std::string& gauge_name)
{
    return ps::BuildGauge().Name(gauge_name).Register(*Prometheus::get_registry());
}

Counter
TickerMetricsPusher::create_counter_(const std::string& gauge_name)
{
    return ps::BuildCounter().Name(gauge_name).Register(*Prometheus::get_registry());
}

void
TickerMetricsPusher::report_orders(const std::vector<tagged_limit_order>& orders)
{
    auto log_order = [&](const tagged_limit_order& order) {
        orders_quantity_counter
            .Add({
                {"ticker",      shared::to_string(order.ticker)},
                {"trader_type", order.trader->get_type()       }
        })
            .Increment(double{order.quantity});
    };

    std::for_each(orders.begin(), orders.end(), log_order);
}

void
TickerMetricsPusher::report_ticker_stats(TickerMapping& tickers)
{
    auto log_midprice = [&](shared::Ticker ticker, const ticker_info& info) {
        ticker_midprice_gauge
            .Add({
                {"ticker", shared::to_string(ticker)}
        })
            .Set(double{info.limit_orderbook.get_midprice()});
    };
    auto log_best_ba = [&](shared::Ticker ticker, ticker_info& info) {
        auto best_bid = info.limit_orderbook.get_top_order(shared::Side::buy);
        auto best_ask = info.limit_orderbook.get_top_order(shared::Side::sell);

        if (best_bid.has_value()) [[likely]] {
            best_ba_gauge
                .Add({
                    {"ticker", shared::to_string(ticker)},
                    {"type",   "BID"                    }
            })
                .Set(double{(**best_bid).price});
        }

        if (best_ask.has_value()) [[likely]] {
            best_ba_gauge
                .Add({
                    {"ticker", shared::to_string(ticker)},
                    {"type",   "ASK"                    }
            })
                .Set(double{(**best_ask).price});
        }
    };

    // auto log_variance = [&](shared::Ticker ticker, const ticker_info& info) {
    //     ticker_midprice_variance_gauge
    //         .Add({
    //             {"ticker", shared::to_string(ticker)}
    //     })
    //         .Set(info.bot_container.get_variance());
    // };

    for (auto [ticker, info] : tickers) {
        log_midprice(ticker, info);
        log_best_ba(ticker, info);
        // log_variance(ticker, info);
    }
}

void
TickerMetricsPusher::report_matches(const std::vector<shared::match>& orders)
{
    auto log_match = [this](const shared::match& match) {
        matches_quantity_counter
            .Add({
                {"ticker", shared::to_string(match.position.ticker)}
        })
            .Increment(double{match.position.quantity});
    };

    std::for_each(orders.begin(), orders.end(), log_match);
}

void
TickerMetricsPusher::report_current_tick(uint64_t tick_num)
{
    current_tick_gauge.Add({}).Set(static_cast<double>(tick_num));
}

void
TickerMetricsPusher::report_trader_stats(const TickerMapping& tickers)
{
    auto report_holdings = [&](const auto& trader) {
        for (auto [ticker, info] : tickers) {
            double amount_held{trader.get_holdings(ticker)};
            per_trader_holdings_gauge
                .Add({
                    {"ticker",      shared::to_string(ticker)},
                    {"trader_type", trader.get_type()        },
                    {"id",          trader.get_id()          },
            })
                .Set(amount_held);
        }
    };

    auto portfolio_value = [&](const auto& trader) {
        double pnl = 0.0;
        for (auto [ticker, info] : tickers) {
            double amount_held{trader.get_holdings(ticker)};
            double midprice{info.limit_orderbook.get_midprice()};
            pnl += amount_held * midprice;
        }
        return pnl;
    };
    auto track_trader = [&](GenericTrader& trader) {
        report_holdings(trader);

        double capital{trader.get_capital()};
        double pnl{
            trader.get_capital() + portfolio_value(trader)
            - trader.get_initial_capital()
        };

        per_trader_pnl_gauge
            .Add({
                {"trader_type", trader.get_type()},
                {"id",          trader.get_id()  },
        })
            .Set(pnl);
        per_trader_capital_gauge
            .Add({
                {"trader_type", trader.get_type()},
                {"id",          trader.get_id()  },
        })
            .Set(capital);
    };

    std::for_each(trader_container_.begin(), trader_container_.end(), track_trader);
}

} // namespace nutc::exchange
