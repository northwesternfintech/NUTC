#include "on_tick_metrics.hpp"

#include "common/types/messages/messages_wrapper_to_exchange.hpp"
#include "exchange/metrics/prometheus.hpp"
#include "exchange/orders/ticker_data.hpp"
#include "exchange/traders/portfolio/trader_portfolio.hpp"
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
TickerMetricsPusher::create_counter_(const std::string& counter_name)
{
    return ps::BuildCounter().Name(counter_name).Register(*Prometheus::get_registry());
}

void
TickerMetricsPusher::report_orders(const std::vector<OrderVariant>& orders)
{
    auto log_order = [&]<typename OrderT>(const OrderT& order) {
        if constexpr (std::is_same_v<OrderT, common::cancel_order>) {
            cancellation_counter
                .Add({
                    {"ticker", common::to_string(order.ticker)}
            })
                .Increment(1);
        }
        else {
            orders_quantity_counter
                .Add({
                    {"ticker",      common::to_string(order.ticker)},
                    {"trader_type", order.trader->get_type()       }
            })
                .Increment(double{order.quantity});
        }
    };

    std::ranges::for_each(orders, [&](const auto& order) {
        std::visit(log_order, order);
    });
}

void
TickerMetricsPusher::report_ticker_stats(TickerContainer& tickers)
{
    auto log_midprice = [&](common::Ticker ticker, const TickerData& info) {
        ticker_midprice_gauge
            .Add({
                {"ticker", common::to_string(ticker)}
        })
            .Set(double{info.get_orderbook().get_midprice()});
    };
    auto log_best_ba = [&](common::Ticker ticker, TickerData& info) {
        auto best_bid = info.get_orderbook().get_top_order(common::Side::buy);
        auto best_ask = info.get_orderbook().get_top_order(common::Side::sell);

        if (best_bid.has_value()) [[likely]] {
            best_ba_gauge
                .Add({
                    {"ticker", common::to_string(ticker)},
                    {"type",   "BID"                    }
            })
                .Set(double{(**best_bid).price});
        }

        if (best_ask.has_value()) [[likely]] {
            best_ba_gauge
                .Add({
                    {"ticker", common::to_string(ticker)},
                    {"type",   "ASK"                    }
            })
                .Set(double{(**best_ask).price});
        }
    };

    auto log_theo = [&](common::Ticker ticker, TickerData& info) {
        ticker_theo
            .Add({
                {"ticker", common::to_string(ticker)}
        })
            .Set(double{(info.get_theo())});
    };

    // auto log_variance = [&](common::Ticker ticker, const TickerData& info) {
    //     ticker_midprice_variance_gauge
    //         .Add({
    //             {"ticker", common::to_string(ticker)}
    //     })
    //         .Set(info.bot_container.get_variance());
    // };

    for (auto [ticker, info] : tickers) {
        log_midprice(ticker, info);
        log_best_ba(ticker, info);
        log_theo(ticker, info);
        // log_variance(ticker, info);
    }
}

void
TickerMetricsPusher::report_matches(const std::vector<common::match>& orders)
{
    auto log_match = [this](const common::match& match) {
        matches_quantity_counter
            .Add({
                {"ticker",     common::to_string(match.position.ticker)},
                {"match_type", match.match_type                        }
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
TickerMetricsPusher::report_trader_stats(const TickerContainer& tickers)
{
    auto report_holdings = [&](const auto& trader) {
        for (auto [ticker, info] : tickers) {
            double amount_held{trader.get_portfolio().get_holdings(ticker)};
            per_trader_holdings_gauge
                .Add({
                    {"ticker",      common::to_string(ticker)},
                    {"trader_type", trader.get_type()        },
                    {"id",          trader.get_id()          },
            })
                .Set(amount_held);
        }
    };

    auto portfolio_value = [&](const TraderPortfolio& portfolio) {
        double pnl = 0.0;
        for (auto [ticker, info] : tickers) {
            double amount_held{portfolio.get_holdings(ticker)};
            double midprice{info.get_orderbook().get_midprice()};
            pnl += amount_held * midprice;
        }
        return pnl;
    };

    auto calculate_pnl = [&](const TraderPortfolio& portfolio) {
        return portfolio.get_capital_delta() + portfolio_value(portfolio);
    };
    auto track_trader = [&](GenericTrader& trader) {
        report_holdings(trader);
        auto& portfolio = trader.get_portfolio();

        double capital{portfolio.get_capital()};
        double pnl{calculate_pnl(portfolio)};

        per_trader_pnl_gauge
            .Add({
                {"id",          trader.get_id()  },
                {"trader_type", trader.get_type()},
        })
            .Set(pnl);
        per_trader_capital_gauge
            .Add({
                {"id",          trader.get_id()  },
                {"trader_type", trader.get_type()},
        })
            .Set(capital);
    };

    std::for_each(trader_container_.begin(), trader_container_.end(), track_trader);
}

} // namespace nutc::exchange
