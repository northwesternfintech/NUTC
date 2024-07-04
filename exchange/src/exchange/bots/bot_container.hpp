#pragma once
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/theo/brownian.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "variance.hpp"

namespace nutc {

namespace bots {

using BotVector = std::vector<std::shared_ptr<traders::BotTrader>>;

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

    const auto&
    get_bots() const
    {
        return bots_;
    }

    BotContainer(
        util::Ticker ticker, double starting_price, std::vector<config::bot_config> bots
    ) :
        ticker(ticker), theo_generator_(starting_price),
        bots_(add_bots(std::move(bots)))
    {}

    double
    get_theo() const
    {
        return theo_generator_.get_magnitude();
    }

	double get_variance() const {
		return variance_calculator_.calculate_volatility();
	}

private:
    void generate_orders(double midprice, double new_theo, double variance);
    BotVector add_bots(const std::vector<config::bot_config>& bot_config);

    template <class BotType>
    BotVector add_bots(double mean_capital, double stddev_capital, size_t num_bots);
};
} // namespace bots
} // namespace nutc
