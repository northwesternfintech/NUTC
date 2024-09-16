#pragma once
#include "common/types/decimal.hpp"
#include "exchange/bots/bot_container.hpp"
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "exchange/traders/trader_container.hpp"

#include <absl/hash/hash.h>

namespace nutc::exchange {

/**
 * @brief Contains the canonical reference to all data coupled to a ticker. Very useful
 * because we typically have to access all at once
 */
// TODO: rename
struct ticker_info {
    CompositeOrderBook limit_orderbook;
    std::vector<BotContainer> bot_containers;

    ticker_info(common::Ticker ticker) :
        limit_orderbook(ticker)
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
        TraderContainer& trader_container, common::Ticker ticker,
        common::decimal_price starting_price, const std::vector<bot_config>& configs
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
