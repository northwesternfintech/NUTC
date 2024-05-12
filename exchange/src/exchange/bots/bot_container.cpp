#include "bot_container.hpp"

#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_container.hpp"

#include <cmath>
#include <cstdint>

#include <random>

namespace nutc {

namespace bots {
void
BotContainer::on_tick(uint64_t)
{
    auto& manager = engine_manager::EngineManager::get_instance();
    auto midprice = manager.get_midprice(TICKER);

    auto theo = fabs(theo_generator_.generate_next_magnitude());
    BotContainer::generate_orders(midprice, theo);
}

template <class BotType>
void
BotContainer::add_bots(double mean_capital, double stddev_capital, size_t num_bots)
requires HandledBotType<BotType>
{
    traders::TraderContainer& users = nutc::traders::TraderContainer::get_instance();

    std::random_device rand;
    std::mt19937 gen(rand());
    std::normal_distribution<> distr(mean_capital, stddev_capital);
    for (size_t i = 0; i < num_bots; i++) {
        auto capital = distr(gen);
        auto bot = users.add_trader<BotType>(TICKER, std::fabs(capital));
        bots_.push_back(bot);
    }
}

template void BotContainer::add_bots<RetailBot>(double, double, size_t);
template void BotContainer::add_bots<MarketMakerBot>(double, double, size_t);
template void BotContainer::add_bots<BollingerBot>(double, double, size_t);

void
BotContainer::generate_orders(double midprice, double new_theo)
{
    for (const auto& bot : bots_) {
        bot->take_action(midprice, new_theo);
    }
}
} // namespace bots
} // namespace nutc
