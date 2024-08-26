#pragma once
#include "exchange/bots/bot_container.hpp"
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/matching/engine.hpp"
#include "exchange/orders/orderbook/cancellable_orderbook.hpp"
#include "exchange/orders/orderbook/level_tracked_orderbook.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/types/decimal_price.hpp"

#include <absl/hash/hash.h>

namespace nutc::exchange {

using DecoratedLimitOrderBook =
    LevelTrackedOrderbook<CancellableOrderBook<LimitOrderBook>>;

/**
 * @brief Contains the canonical reference to all data coupled to a ticker. Very useful
 * because we typically have to access all at once
 */
// TODO: rename
struct ticker_info {
    DecoratedLimitOrderBook limit_orderbook;
    Engine engine;
    std::vector<BotContainer> bot_containers;

    ticker_info(shared::Ticker ticker, double order_fee) :
        limit_orderbook(ticker), engine(order_fee)
    {}

    void
    set_bot_config(TraderContainer& traders, const ticker_config& config)
    {
        bot_containers = create_bot_containers(
            traders, config.TICKER, config.STARTING_PRICE, config.BOTS
        );
    }

private:
    std::vector<BotContainer>
    create_bot_containers(
        TraderContainer& trader_container, shared::Ticker ticker,
        shared::decimal_price starting_price, const std::vector<bot_config>& configs
    )
    {
        std::vector<BotContainer> containers;
        containers.reserve(configs.size());
        for (const bot_config& bot_config : configs) {
            containers.emplace_back(
                ticker, starting_price, trader_container, bot_config
            );
        }
        return containers;
    }
};

} // namespace nutc::exchange
