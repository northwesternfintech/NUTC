#pragma once
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/theo/brownian.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared/types/decimal_price.hpp"
#include "shared_bot_state.hpp"
#include "variance.hpp"

namespace nutc::exchange {

/**
 * @brief Container for bots for a given ticker
 */
class BotContainer {
    using BotVector = std::vector<std::shared_ptr<BotTrader>>;
    shared::Ticker ticker;
    BrownianMotion theo_generator_;
    VarianceCalculator variance_calculator_;

    BotVector bots_{};

public:
    void generate_orders(shared::decimal_price midprice);

    BotContainer(
        shared::Ticker ticker, shared::decimal_price starting_price,
        TraderContainer& trader_container, bot_config bots
    ) :
        ticker(ticker),
        theo_generator_(starting_price),
        bots_(create_bots(trader_container, ticker, bots))
    {}

    shared::decimal_price
    get_theo() const
    {
        return theo_generator_.get_magnitude();
    }

    double
    get_variance() const
    {
        return variance_calculator_.calculate_volatility();
    }

private:
    void generate_orders(const shared_bot_state& shared_state);

    static BotVector create_bots(
        TraderContainer& traders, shared::Ticker ticker, const bot_config& bot_config
    );

    template <class BotType>
    static BotVector create_bots(
        TraderContainer& trader_container, shared::Ticker ticker,
        shared::decimal_price mean_capital, shared::decimal_price stddev_capital,
        size_t num_bots
    );
};
} // namespace nutc::exchange
