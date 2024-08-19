#pragma once

#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/config/static/config.hpp"
#include "exchange/theo/random_noise_generator.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared/types/decimal_price.hpp"

#include <random>

namespace nutc {

namespace bots {
using util::decimal_price;

template <typename RandomNoiseGenerator = stochastic::RandomNoiseGenerator>
class RetailBot : public traders::BotTrader {
    std::mt19937 gen_{}; // NOLINT
    std::poisson_distribution<> poisson_dist_{};

    decimal_price
    generate_aggresiveness_()
    {
        static std::normal_distribution<> dist{1000, 2000};
        return dist(gen_);
    }

public:
    RetailBot(util::Ticker ticker, decimal_price interest_limit) :
        BotTrader(ticker, interest_limit), AGGRESSIVENESS(generate_aggresiveness_())
    {}

    void
    take_action(const shared_bot_state& state) override
    {
        static std::uniform_real_distribution<> dis{0.0, 1.0};

        util::decimal_price p_trade =
            (util::decimal_price{1.0} - get_capital_utilization());

        double noise_factor = dis(gen_);

        util::decimal_price signal_strength =
            AGGRESSIVENESS * state.THEO.difference(state.MIDPRICE) / state.MIDPRICE;

        if (noise_factor >= p_trade * signal_strength)
            return;
        if (poisson_dist_(gen_) <= 0)
            return;

        util::decimal_price noised_theo =
            state.THEO + RandomNoiseGenerator::generate(0, .1);
        double quantity{
            (util::decimal_price{1.0} - get_capital_utilization())
            * get_interest_limit() / noised_theo
        };
        quantity *= RETAIL_ORDER_SIZE;

        auto side = (state.MIDPRICE < noised_theo) ? util::Side::buy : util::Side::sell;

        add_market_order(side, quantity);
    }

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "RETAIL";
        return TYPE;
    }

private:
    const decimal_price AGGRESSIVENESS;
};

} // namespace bots

} // namespace nutc
