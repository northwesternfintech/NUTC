#include "bot_container.hpp"

#include "exchange/rabbitmq/order_handler/RabbitMQOrderHandler.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_manager.hpp"

#include <cmath>

#include <random>
#include <utility>

double
generate_gaussian_noise(double mean, double stddev)
{
    static std::random_device rand;
    static std::mt19937 gen(rand());
    std::normal_distribution<> distr(mean, stddev); // Define the normal distribution

    return distr(gen);
}

namespace nutc {

namespace bots {
void
BotContainer::on_tick(uint64_t)
{
    auto theo = fabs(theo_generator_.generate_next_price() + brownian_offset_);
    auto& ticker = engine_manager::EngineManager::get_instance().get_engine(ticker_);
    double current = ticker.get_midprice();
    auto orders = BotContainer::on_new_theo(theo, current);

    for (auto& order : orders) {
        order.ticker = ticker_;
        rabbitmq::RabbitMQOrderHandler::handle_incoming_market_order(
            engine_manager::EngineManager::get_instance(),
            manager::ClientManager::get_instance(), std::move(order)
        );
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
    manager::ClientManager& users = nutc::manager::ClientManager::get_instance();

    BotType bot(ticker_, starting_capital);
    std::string bot_id = bot.get_id();
    auto retail_bot = users.add_bot_trader(std::move(bot));
    if constexpr (std::is_same_v<BotType, RetailBot>) {
        retail_bots_.insert({bot_id, retail_bot});
    }
    else {
        market_makers_.insert({bot_id, retail_bot});
    }
}

template void BotContainer::add_single_bot_<RetailBot>(double);
template void BotContainer::add_single_bot_<MarketMakerBot>(double);

std::vector<MarketOrder>
BotContainer::on_new_theo(double new_theo, double current)
{
    auto mm_new_theo = [new_theo](auto& mm_trader, std::vector<MarketOrder>& orders) {
        double noised_theo =
            new_theo + static_cast<double>(generate_gaussian_noise(0, .02));

        std::vector<messages::MarketOrder> mm_orders =
            mm_trader->take_action(noised_theo);
        orders.insert(orders.end(), mm_orders.begin(), mm_orders.end());
    };

    auto retail_new_theo =
        [new_theo, current](auto& retail_trader, std::vector<MarketOrder>& orders) {
            double noised_theo =
                new_theo + static_cast<double>(generate_gaussian_noise(0, .1));
            auto bot_order = retail_trader->take_action(current, noised_theo);
            if (bot_order.has_value())
                orders.push_back(bot_order.value());
        };

    std::vector<MarketOrder> orders{};
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
