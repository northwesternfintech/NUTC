#pragma once
#include "common/types/decimal.hpp"
#include "exchange/bots/bot_container.hpp"
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "exchange/theo/brownian.hpp"
#include "exchange/traders/trader_container.hpp"

#include <absl/hash/hash.h>

namespace nutc::exchange {

/**
 * @brief Contains the canonical reference to all data coupled to a ticker. Very useful
 * because we typically have to access all at once
 */
class TickerData {
    CompositeOrderBook limit_orderbook_;
    BrownianMotion theo_generator_;
    std::vector<BotContainer> bot_containers_;

public:
    explicit TickerData(common::Ticker ticker) : limit_orderbook_{ticker} {}

    common::decimal_price
    get_theo() const
    {
        return theo_generator_.get_magnitude();
    }

    CompositeOrderBook&
    get_orderbook()
    {
        return limit_orderbook_;
    }

    const CompositeOrderBook&
    get_orderbook() const
    {
        return limit_orderbook_;
    }

    void
    generate_bot_orders()
    {
        auto midprice = get_orderbook().get_midprice();
		auto theo = theo_generator_.generate_next_magnitude();
        std::ranges::for_each(bot_containers_, [&](auto& bot_container) {
            bot_container.generate_orders(midprice);
        });
    }

    void
    set_bot_config(TraderContainer& traders, const ticker_config& config)
    {
        bot_containers_ = create_bot_containers(
            traders, config.TICKER, config.STARTING_PRICE, config.BOTS
        );
    }

private:
    static std::vector<BotContainer>
    create_bot_containers(
        TraderContainer& trader_container, common::Ticker ticker,
        common::decimal_price starting_price, const std::vector<bot_config>& configs
    )
    {
        std::vector<BotContainer> bot_containers;
        bot_containers.reserve(configs.size());
        for (const bot_config& bot_config : configs) {
            bot_containers.emplace_back(
                ticker, starting_price, trader_container, bot_config
            );
        }
        return bot_containers;
    }
};

} // namespace nutc::exchange
