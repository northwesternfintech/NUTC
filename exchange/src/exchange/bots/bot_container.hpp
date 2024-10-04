#pragma once
#include "common/types/decimal.hpp"
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared_bot_state.hpp"
#include "variance.hpp"

namespace nutc::exchange {

/**
 * @brief Container for bots for a given ticker
 */
class BotContainer {
    using BotVector = std::vector<std::shared_ptr<BotTrader>>;
    VarianceCalculator variance_calculator_;

    BotVector bots_{};

public:
    void generate_orders(common::decimal_price midprice, common::decimal_price theo);

    BotContainer(
        common::Ticker ticker, TraderContainer& trader_container, bot_config bots
    ) : bots_(create_bots(trader_container, ticker, bots))
    {}

    double
    get_variance() const
    {
        return variance_calculator_.calculate_volatility();
    }

private:
    void generate_orders(const shared_bot_state& shared_state);

    static BotVector create_bots(
        TraderContainer& traders, common::Ticker ticker, const bot_config& bot_config
    );

    template <class BotType>
    static BotVector create_bots(
        TraderContainer& trader_container, common::Ticker ticker,
        common::decimal_price mean_capital, common::decimal_price stddev_capital,
        size_t num_bots
    );
};
} // namespace nutc::exchange
