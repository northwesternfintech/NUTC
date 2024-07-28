#include "bot_container.hpp"

#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/bots/bot_types/retail.hpp"
#include "exchange/traders/trader_container.hpp"

#include <cmath>

#include <iterator>
#include <random>

namespace nutc {

namespace bots {
template <typename T>
concept HandledBotType =
    std::disjunction_v<std::is_same<T, RetailBot>, std::is_same<T, MarketMakerBot>>;

void
BotContainer::generate_orders(double midprice)
{
    auto theo = fabs(theo_generator_.generate_next_magnitude());
    variance_calculator_.record_price(midprice);
    generate_orders(midprice, theo, variance_calculator_.calculate_volatility());
}

template <class BotType>
BotVector
BotContainer::add_bots(double mean_capital, double stddev_capital, size_t num_bots)
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
BotContainer::add_bots(const std::vector<config::bot_config>& bot_config)
{
    BotVector bot_vec;
    for (auto& bots : bot_config) {
        switch (bots.TYPE) {
            case config::BotType::retail:
                std::ranges::move(
                    add_bots<RetailBot>(
                        bots.AVERAGE_CAPITAL, bots.STD_DEV_CAPITAL, bots.NUM_BOTS
                    ),
                    std::back_inserter(bot_vec)
                );
                break;
            case config::BotType::market_maker:
                std::ranges::move(
                    add_bots<MarketMakerBot>(
                        bots.AVERAGE_CAPITAL, bots.STD_DEV_CAPITAL, bots.NUM_BOTS
                    ),
                    std::back_inserter(bot_vec)
                );
        }
    }
    return bot_vec;
}

void
BotContainer::generate_orders(double midprice, double new_theo, double variance)
{
    for (const auto& bot : bots_) {
        bot->take_action(midprice, new_theo, variance);
    }
}
} // namespace bots
} // namespace nutc
