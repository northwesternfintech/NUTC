#include "bot_container.hpp"

#include "exchange/rabbitmq/order_handler/RabbitMQOrderHandler.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_manager.hpp"

#include <cmath>

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
    auto current = engine_manager::EngineManager::get_instance().get_engine(ticker_);
    assert(current.has_value());
    auto current_price = current.value().get().get_midprice();
    auto orders = BotContainer::on_new_theo(static_cast<float>(theo), current_price);

    for (auto& order : orders) {
        order.ticker = ticker_;
        rabbitmq::RabbitMQOrderHandler::handle_incoming_market_order(
            engine_manager::EngineManager::get_instance(),
            manager::ClientManager::get_instance(), std::move(order)
        );
    }
}

void
BotContainer::add_mm_bot(float starting_capital)
{
    manager::ClientManager& users = nutc::manager::ClientManager::get_instance();
    std::string bot_id = users.add_client(manager::bot_trader_t{});

    market_makers_[bot_id] = MarketMakerBot(bot_id, starting_capital);
    manager::trader_t& bot = users.get_client(bot_id);
    assert(std::holds_alternative<manager::bot_trader_t>(bot));
    auto& bot_trader = std::get<manager::bot_trader_t>(bot);
    bot_trader.set_capital(starting_capital);
    bot_trader.modify_holdings(ticker_, INFINITY);
}

std::vector<MarketOrder>
BotContainer::on_new_theo(float new_theo, float current)
{
    std::vector<MarketOrder> orders;
    for (auto& [id, bot] : market_makers_) {
        float noised_theo = new_theo + generate_gaussian_noise(0, .02);
        std::vector<messages::MarketOrder> bot_orders = bot.take_action(noised_theo);
        orders.insert(orders.end(), bot_orders.begin(), bot_orders.end());
    }
    return orders;
}

void
BotContainer::process_bot_match(const Match& match)
{
    auto match1 = market_makers_.find(match.buyer_id);
    auto match2 = market_makers_.find(match.seller_id);
    float total_cap = match.price * match.quantity;

    // Both have reduced their positions
    if (match1 != market_makers_.end()) {
        match1->second.modify_long_capital(-total_cap);
    }
    if (match2 != market_makers_.end()) {
        match2->second.modify_short_capital(-total_cap);
    }
}

void
BotContainer::process_order_expiration(
    const std::string& bot_id, messages::SIDE side, float total_cap
)
{
    auto match1 = market_makers_.find(bot_id);

    // Both have reduced their positions
    assert(match1 != market_makers_.end());
    if (side == messages::SIDE::BUY) {
        match1->second.modify_long_capital(-total_cap);
    }
    else {
        match1->second.modify_short_capital(-total_cap);
    }
}

} // namespace bots
} // namespace nutc
