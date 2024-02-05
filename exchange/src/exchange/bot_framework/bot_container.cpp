#include "bot_container.hpp"

#include "exchange/matching/manager/engine_manager.hpp"
#include "exchange/rabbitmq/order_handler/RabbitMQOrderHandler.hpp"

#include <cmath>

namespace nutc {

namespace bots {
void
BotContainer::on_tick(uint64_t)
{
    auto theo = theo_generator_.generate_next_price();
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
BotContainer::add_mm_bot(const std::string& bot_id, float starting_capital)
{
    std::string new_bot_id = "bot_" + bot_id;
    market_makers_[new_bot_id] = MarketMakerBot(new_bot_id, starting_capital);

    manager::ClientManager& users = nutc::manager::ClientManager::get_instance();
    users.add_client(new_bot_id, ticker_, nutc::manager::ClientLocation::BOT);
    users.modify_capital(new_bot_id, starting_capital);
    users.modify_holdings(new_bot_id, ticker_, INFINITY);
}

std::vector<MarketOrder>
BotContainer::on_new_theo(float new_theo, float current)
{
    std::vector<MarketOrder> orders;
    for (auto& [id, bot] : market_makers_) {
        std::vector<messages::MarketOrder> bot_orders = bot.take_action(new_theo);
        orders.insert(orders.end(), bot_orders.begin(), bot_orders.end());
    }
    return orders;
}

void
BotContainer::process_bot_match(const Match& match)
{
    auto match1 = market_makers_.find(match.buyer_id);
    auto match2 = market_makers_.find(match.seller_id);
    log_i(main, "Processing bot match");
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
    log_i(main, "Processing bot cancellation {}", bot_id);
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
