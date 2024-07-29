#include "bot_container.hpp"

#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/bots/bot_types/retail.hpp"
#include "exchange/traders/trader_container.hpp"

#include <cmath>

#include <random>

namespace nutc::bots {

void
BotContainer::generate_orders(double midprice)
{
    auto theo = fabs(theo_generator_.generate_next_magnitude());
    variance_calculator_.record_price(midprice);

    double cumulative_interest_limit = 0;
    double cumulative_quantity_held = 0;

    for (const auto& bot : bots_) {
        cumulative_interest_limit += bot->get_interest_limit();
        cumulative_quantity_held += bot->get_holdings();
    }

    return generate_orders(
        {midprice, theo, variance_calculator_.calculate_volatility(),
         cumulative_interest_limit, cumulative_quantity_held}
    );
}

template <class BotType>
BotVector
BotContainer::create_bots(double mean_capital, double stddev_capital, size_t num_bots)
{
    BotVector bot_vec;
    traders::TraderContainer& users = nutc::traders::TraderContainer::get_instance();

    std::random_device rand;
    std::mt19937 gen(rand());
    std::normal_distribution<> distr(mean_capital, stddev_capital);
    for (size_t i = 0; i < num_bots; i++) {
        auto capital = distr(gen);
        auto bot = users.add_trader<BotType>(ticker, std::fabs(capital));
        bot_vec.push_back(bot);
    }
    return bot_vec;
}

BotVector
BotContainer::create_bots(const config::bot_config& bot_config)
{
    switch (bot_config.TYPE) {
        case config::BotType::retail:
            return create_bots<RetailBot>(
                bot_config.AVERAGE_CAPITAL, bot_config.STD_DEV_CAPITAL,
                bot_config.NUM_BOTS
            );
        case config::BotType::market_maker:
            return create_bots<MarketMakerBot>(
                bot_config.AVERAGE_CAPITAL, bot_config.STD_DEV_CAPITAL,
                bot_config.NUM_BOTS
            );
    }

    throw std::runtime_error("Unknown bot type");
}

void
BotContainer::generate_orders(const shared_bot_state& shared_state)
{
    for (const auto& bot : bots_) {
        bot->take_action(shared_state);
    }
}
} // namespace nutc::bots
