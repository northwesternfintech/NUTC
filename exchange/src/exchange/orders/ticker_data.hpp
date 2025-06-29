#pragma once
#include "common/types/decimal.hpp"
#include "common/types/ticker.hpp"
#include "exchange/bots/bot_container.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "exchange/theo/brownian.hpp"

namespace nutc::exchange {

/**
 * @brief Contains the canonical reference to all data coupled to a ticker. Very useful
 * because we typically have to access all at once
 */
class TickerData {
    CompositeOrderBook limit_orderbook_;
    BrownianMotion theo_generator_; // NOLINT
    std::vector<BotContainer> bot_containers_;

public:
    explicit TickerData(
        common::Ticker ticker, common::decimal_price starting_price,
        std::vector<BotContainer> bots
    ) :
        limit_orderbook_{ticker}, theo_generator_{starting_price},
        bot_containers_{std::move(bots)}
    {}

    // TODO: check where this is used and if we can get rid of it
    explicit TickerData(common::Ticker ticker) : limit_orderbook_{ticker} {}

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
        auto theo = fabs(theo_generator_.generate_next_magnitude());
        std::ranges::for_each(bot_containers_, [&](auto& bot_container) {
            bot_container.generate_orders(midprice, theo);
        });
    }

    common::decimal_price
    get_theo() const
    {
        return theo_generator_.get_magnitude();
    }
};

} // namespace nutc::exchange
