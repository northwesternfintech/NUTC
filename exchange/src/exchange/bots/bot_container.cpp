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
    auto ticker = engine_manager::EngineManager::get_instance().get_engine(ticker_);
    assert(ticker.has_value());
    double current = ticker.value().get().get_midprice();
    auto orders = BotContainer::on_new_theo(theo, current);

    for (auto& order : orders) {
        order.ticker = ticker_;
        rabbitmq::RabbitMQOrderHandler::handle_incoming_market_order(
            engine_manager::EngineManager::get_instance(),
            manager::ClientManager::get_instance(), std::move(order)
        );
    }
}

void
BotContainer::add_mm_bots(const std::vector<double>& starting_capitals)
{
    for (double capital : starting_capitals) {
        add_mm_bot_(capital);
    }
}

void
BotContainer::add_retail_bots(double mean_capital, double stddev_capital, int num_bots)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> distr(mean_capital, stddev_capital);
    for (int i = 0; i < num_bots; i++) {
        auto capital = distr(gen);
        capital = std::abs(capital);
        add_retail_bot_(capital);
    }
}

void
BotContainer::add_retail_bot_(double starting_capital)
{
    manager::ClientManager& users = nutc::manager::ClientManager::get_instance();
    std::string bot_id = users.add_bot_trader();

    retail_bots_.emplace(
        std::piecewise_construct, std::forward_as_tuple(bot_id),
        std::forward_as_tuple(bot_id, starting_capital)
    );
    const auto& bot = users.get_trader(bot_id);
    assert(bot->get_type() == manager::BOT);
    bot->set_capital(starting_capital);
    bot->modify_holdings(ticker_, INFINITY);
}

void
BotContainer::add_mm_bot_(double starting_capital)
{
    manager::ClientManager& users = nutc::manager::ClientManager::get_instance();
    std::string bot_id = users.add_bot_trader();

    market_makers_.emplace(
        std::piecewise_construct, std::forward_as_tuple(bot_id),
        std::forward_as_tuple(bot_id, starting_capital)
    );
    const auto& bot = users.get_trader(bot_id);
    assert(bot->get_type() == manager::BOT);
    bot->set_capital(starting_capital);
    bot->modify_holdings(ticker_, INFINITY);
}

std::vector<MarketOrder>
BotContainer::on_new_theo(double new_theo, double current)
{
    auto mm_new_theo = [new_theo](auto&& mm_trader, std::vector<MarketOrder>& orders) {
        double noised_theo =
            new_theo + static_cast<double>(generate_gaussian_noise(0, .02));
        std::vector<messages::MarketOrder> mm_orders =
            mm_trader.take_action(noised_theo);
        orders.insert(orders.end(), mm_orders.begin(), mm_orders.end());
    };

    auto retail_new_theo =
        [new_theo, current](auto&& retail_trader, std::vector<MarketOrder>& orders) {
            double noised_theo =
                new_theo + static_cast<double>(generate_gaussian_noise(0, .1));
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
BotContainer::process_order_match(Match& match)
{
    auto process_buyer_match = [&match](auto&& umap) {
        auto buyer_match = umap.find(match.buyer_id);
        if (buyer_match == umap.end())
            return;
        buyer_match->second.modify_held_stock(1);
        buyer_match->second.modify_capital(-match.quantity * match.price);
    };

    auto process_seller_match = [&match](auto&& umap) {
        auto seller_match = umap.find(match.seller_id);
        if (seller_match == umap.end())
            return;
        seller_match->second.modify_held_stock(-1);
        seller_match->second.modify_capital(match.quantity * match.price);
    };

    process_buyer_match(market_makers_);
    process_buyer_match(retail_bots_);
    process_seller_match(market_makers_);
    process_seller_match(retail_bots_);
}

void
BotContainer::process_order_add(
    const std::string& bot_id, messages::SIDE side, double total_cap
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
    const std::string& bot_id, messages::SIDE side, double total_cap
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
