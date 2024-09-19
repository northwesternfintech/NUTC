#include "bot_container.hpp"

#include "common/types/decimal.hpp"
#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/bots/bot_types/retail.hpp"
#include "exchange/traders/trader_container.hpp"

#include <cmath>

#include <random>

namespace nutc::exchange {

void
BotContainer::generate_orders(
    common::decimal_price midprice, common::decimal_price theo
)
{
    variance_calculator_.record_price(midprice);

    decimal_price cumulative_interest_limit{};
    common::decimal_quantity cumulative_quantity_held{};

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
BotContainer::BotVector
BotContainer::create_bots(
    TraderContainer& trader_container, common::Ticker ticker,
    decimal_price mean_capital, decimal_price stddev_capital, size_t num_bots
)
{
    BotVector bot_vec;

    std::random_device rand;
    std::mt19937 gen(rand());
    std::normal_distribution<> distr(double{mean_capital}, double{stddev_capital});
    for (size_t i = 0; i < num_bots; i++) {
        auto capital = distr(gen);
        auto bot = trader_container.add_trader<BotType>(ticker, std::fabs(capital));
        bot_vec.push_back(bot);
    }
    return bot_vec;
}

BotContainer::BotVector
BotContainer::create_bots(
    TraderContainer& traders, common::Ticker ticker, const bot_config& bot_config
)
{
    switch (bot_config.TYPE) {
        case BotType::retail:
            return create_bots<RetailBot>(
                traders, ticker, bot_config.AVERAGE_CAPITAL, bot_config.STD_DEV_CAPITAL,
                bot_config.NUM_BOTS
            );
        case BotType::market_maker:
            return create_bots<MarketMakerBot>(
                traders, ticker, bot_config.AVERAGE_CAPITAL, bot_config.STD_DEV_CAPITAL,
                bot_config.NUM_BOTS
            );
    }

    throw std::invalid_argument("Unknown bot type");
}

void
BotContainer::generate_orders(const shared_bot_state& shared_state)
{
    for (const auto& bot : bots_) {
        bot->take_action(shared_state);
    }
}
} // namespace nutc::exchange
