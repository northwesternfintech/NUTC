#include "bot_container.hpp"

#include "exchange/rabbitmq/order_handler/RabbitMQOrderHandler.hpp"
#include "exchange/tick_manager/tick_manager.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_manager.hpp"

#include <cmath>

#include <random>
#include <utility>

double
generate_gaussian_noise(double mean, double stddev)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> distr(mean, stddev); // Define the normal distribution

    return distr(gen);
}

namespace nutc {

namespace bots {
void
BotContainer::on_tick(uint64_t)
{
    auto theo = fabs(theo_generator_.generate_next_price() + brownian_offset_);
    float current = engine_manager::EngineManager::get_instance()
                        .get_engine(ticker_)
                        .value()
                        .get()
                        .get_midprice();
    auto orders = BotContainer::on_new_theo(static_cast<float>(theo), current);

    for (auto& order : orders) {
        order.ticker = ticker_;
        rabbitmq::RabbitMQOrderHandler::handle_incoming_market_order(
            engine_manager::EngineManager::get_instance(),
            manager::ClientManager::get_instance(), std::move(order)
        );
    }
}

void
BotContainer::add_mm_bots(const std::vector<float>& starting_capitals)
{
    for (float capital : starting_capitals) {
        add_mm_bot_(capital);
    }
}

void
BotContainer::add_retail_bots(float mean_capital, float stddev_capital, int num_bots)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> distr(mean_capital, stddev_capital);
    for (int i = 0; i < num_bots; i++) {
        auto capital = static_cast<float>(distr(gen));
        capital = std::abs(capital);
        add_retail_bot_(capital);
    }
}

void
BotContainer::add_retail_bot_(float starting_capital)
{
    manager::ClientManager& users = nutc::manager::ClientManager::get_instance();
    std::string bot_id = users.add_client(manager::bot_trader_t{});

    retail_bots_.emplace(
        std::piecewise_construct, std::forward_as_tuple(bot_id),
        std::forward_as_tuple(bot_id, starting_capital)
    );
    manager::trader_t& bot = users.get_client(bot_id);
    assert(std::holds_alternative<manager::bot_trader_t>(bot));
    auto& bot_trader = std::get<manager::bot_trader_t>(bot);
    bot_trader.set_capital(starting_capital);
    bot_trader.modify_holdings(ticker_, INFINITY);
}

void
BotContainer::add_mm_bot_(float starting_capital)
{
    manager::ClientManager& users = nutc::manager::ClientManager::get_instance();
    std::string bot_id = users.add_client(manager::bot_trader_t{});

    market_makers_.emplace(
        std::piecewise_construct, std::forward_as_tuple(bot_id),
        std::forward_as_tuple(bot_id, starting_capital)
    );
    manager::trader_t& bot = users.get_client(bot_id);
    assert(std::holds_alternative<manager::bot_trader_t>(bot));
    auto& bot_trader = std::get<manager::bot_trader_t>(bot);
    bot_trader.set_capital(starting_capital);
    bot_trader.modify_holdings(ticker_, INFINITY);
}

std::vector<MarketOrder>
BotContainer::on_new_theo(float new_theo, float current)
{
    auto mm_new_theo = [new_theo](auto&& mm_trader, std::vector<MarketOrder>& orders) {
        float noised_theo =
            new_theo + static_cast<float>(generate_gaussian_noise(0, .02));
        std::vector<messages::MarketOrder> mm_orders =
            mm_trader.take_action(noised_theo);
        orders.insert(orders.end(), mm_orders.begin(), mm_orders.end());
    };

    auto retail_new_theo =
        [new_theo, current](auto&& retail_trader, std::vector<MarketOrder>& orders) {
            float noised_theo =
                new_theo + static_cast<float>(generate_gaussian_noise(0, .1));
            auto bot_order = retail_trader.take_action(current, noised_theo);
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

void
BotContainer::process_order_add(
    const std::string& bot_id, messages::SIDE side, float total_cap
)
{
    auto process_order_add = [side, total_cap](auto&& match) {
        if (side == messages::SIDE::BUY) {
            match->second.modify_long_capital(total_cap);
            match->second.modify_open_bids(1);
        }
        else {
            match->second.modify_short_capital(total_cap);
            match->second.modify_open_asks(1);
        }
    };

    auto retail_match = retail_bots_.find(bot_id);
    if (retail_match != retail_bots_.end()) {
        process_order_add(retail_match);
        return;
    }
    auto mm_match = market_makers_.find(bot_id);
    if (mm_match != market_makers_.end()) {
        process_order_add(mm_match);
        return;
    }
    throw std::runtime_error("Bot not found");
}

void
BotContainer::process_order_expiration(
    const std::string& bot_id, messages::SIDE side, float total_cap
)
{
    auto process_order_expiration = [side, total_cap](auto match) {
        if (side == messages::SIDE::BUY) {
            match->second.modify_long_capital(-total_cap);
            match->second.modify_open_bids(-1);
        }
        else {
            match->second.modify_short_capital(-total_cap);
            match->second.modify_open_asks(-1);
        }
    };

    auto retail_match = retail_bots_.find(bot_id);
    if (retail_match != retail_bots_.end()) {
        process_order_expiration(retail_match);
        return;
    }
    auto match1 = market_makers_.find(bot_id);
    if (match1 != market_makers_.end()) {
        process_order_expiration(match1);
        return;
    }
    throw std::runtime_error("Bot not found");
}

} // namespace bots
} // namespace nutc