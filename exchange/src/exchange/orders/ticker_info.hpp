#pragma once
#include "exchange/bots/bot_container.hpp"
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/matching/engine.hpp"
#include "exchange/orders/orderbook/cancellable_orderbook.hpp"
#include "exchange/orders/orderbook/level_tracked_orderbook.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/traders/trader_container.hpp"

#include <absl/hash/hash.h>

namespace nutc {
namespace matching {

template <typename BaseOrderBookT>
using DecoratedOrderBook = LevelTrackedOrderbook<CancellableOrderBook<BaseOrderBookT>>;

/**
 * @brief Contains the canonical reference to all data coupled to a ticker. Very useful
 * because we typically have to access all at once
 */
struct ticker_info {
    DecoratedOrderBook<LimitOrderBook> orderbook{};
    Engine engine;
    std::vector<bots::BotContainer> bot_containers;

    ticker_info(util::decimal_price order_fee) : engine(order_fee) {}

    // TODO: order fee should not be 0
    ticker_info(
        traders::TraderContainer& trader_container, const config::ticker_config& config
    ) :
        ticker_info(
            trader_container, config.TICKER, config.STARTING_PRICE, 0.0, config.BOTS
        )
    {}

    ticker_info(
        traders::TraderContainer& trader_container, util::Ticker ticker,
        double starting_price, util::decimal_price order_fee,
        std::vector<config::bot_config> config
    ) :
        engine(order_fee),
        bot_containers(
            create_bot_containers(trader_container, ticker, starting_price, config)
        )
    {}

private:
    std::vector<bots::BotContainer>
    create_bot_containers(
        traders::TraderContainer& trader_container, util::Ticker ticker,
        double starting_price, const std::vector<config::bot_config>& configs
    )
    {
        std::vector<bots::BotContainer> containers;
        for (const config::bot_config& bot_config : configs) {
            containers.emplace_back(
                ticker, starting_price, trader_container, bot_config
            );
        }
        return containers;
    }
};

using TickerMapping =
    emhash7::HashMap<util::Ticker, ticker_info, absl::Hash<util::Ticker>>;

} // namespace matching
} // namespace nutc
