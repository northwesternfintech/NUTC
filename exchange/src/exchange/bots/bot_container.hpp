#pragma once
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/theo/brownian.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared_bot_state.hpp"
#include "variance.hpp"

namespace nutc {

namespace bots {

using BotVector = std::vector<std::shared_ptr<traders::BotTrader>>;
using traders::TraderContainer;

/**
 * @brief Container for bots for a given ticker
 */
class BotContainer {
    util::Ticker ticker;
    stochastic::BrownianMotion theo_generator_;
    VarianceCalculator variance_calculator_;

    BotVector bots_{};

public:
    void generate_orders(double midprice);

    BotContainer(
        util::Ticker ticker, double starting_price, TraderContainer& trader_container,
        config::bot_config bots
    ) :
        ticker(ticker), theo_generator_(starting_price),
        bots_(create_bots(trader_container, std::move(bots)))
    {}

    double
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

    BotVector create_bots(
        TraderContainer& trader_container, const config::bot_config& bot_config
    );

    template <class BotType>
    BotVector create_bots(
        TraderContainer& trader_container, double mean_capital, double stddev_capital,
        size_t num_bots
    );
};
} // namespace bots
} // namespace nutc
