#include "bot_container.hpp"

#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_container.hpp"

#include <cmath>
#include <cstdint>

#include <random>

namespace {
double
generate_gaussian_noise(double mean, double stddev)
{
    static std::random_device rand{};
    static std::mt19937 gen{rand()};
    static std::normal_distribution<double> distr{mean, stddev};

    return distr(gen);
}
} // namespace

namespace nutc {

namespace bots {
void
BotContainer::on_tick(uint64_t current_tick)
{
    // This should be simpler. We shouldn't need the manager here
    auto& manager = engine_manager::EngineManager::get_instance();
    auto current = manager.get_midprice(TICKER);

    auto theo = fabs(theo_generator_.generate_next_price() + BROWNIAN_OFFSET);
    auto orders = BotContainer::on_new_theo(theo, current, current_tick);

    for (auto& order : orders) {
        assert(order.ticker == TICKER);
        manager.match_order(order);
    }
}

template <class BotType>
void
BotContainer::add_bots(double mean_capital, double stddev_capital, size_t num_bots)
requires HandledBotType<BotType>
{
    std::random_device rand;
    std::mt19937 gen(rand());
    std::normal_distribution<> distr(mean_capital, stddev_capital);
    for (size_t i = 0; i < num_bots; i++) {
        auto capital = distr(gen);
        capital = std::abs(capital);
        add_single_bot_<BotType>(capital);
    }
}

template void BotContainer::add_bots<RetailBot>(double, double, size_t);
template void BotContainer::add_bots<MarketMakerBot>(double, double, size_t);

template <class BotType>
void
BotContainer::add_single_bot_(double starting_capital)
requires HandledBotType<BotType>
{
    traders::TraderContainer& users = nutc::traders::TraderContainer::get_instance();

    auto retail_bot = users.add_trader<BotType>(TICKER, starting_capital);
    if constexpr (std::is_same_v<BotType, RetailBot>) {
        retail_bots_.insert({retail_bot->get_id(), retail_bot});
    }
    else {
        market_makers_.insert({retail_bot->get_id(), retail_bot});
    }
}

template void BotContainer::add_single_bot_<RetailBot>(double);
template void BotContainer::add_single_bot_<MarketMakerBot>(double);

std::vector<matching::stored_order>
BotContainer::on_new_theo(double new_theo, double current, uint64_t current_tick)
{
    auto mm_new_theo = [new_theo, current_tick](
                           auto& mm_trader, std::vector<matching::stored_order>& orders
                       ) {
        double noised_theo = new_theo + generate_gaussian_noise(0, .02);

        std::vector<matching::stored_order> mm_orders =
            mm_trader->take_action(noised_theo, current_tick);

        orders.reserve(orders.size() + mm_orders.size());
        orders.insert(
            orders.end(), std::make_move_iterator(mm_orders.begin()),
            std::make_move_iterator(mm_orders.end())
        );
    };

    auto retail_new_theo = [new_theo, current, current_tick](
                               auto& retail_trader,
                               std::vector<matching::stored_order>& orders
                           ) {
        double noised_theo =
            new_theo + static_cast<double>(generate_gaussian_noise(0, .1));
        auto bot_order = retail_trader->take_action(current, noised_theo, current_tick);
        if (bot_order.has_value())
            orders.push_back(bot_order.value());
    };

    std::vector<matching::stored_order> orders{};
    for (auto& [_, mm_trader] : market_makers_) {
        mm_new_theo(mm_trader, orders);
    }

    for (auto& [_, bot] : retail_bots_) {
        retail_new_theo(bot, orders);
    }

    return orders;
}
} // namespace bots
} // namespace nutc
