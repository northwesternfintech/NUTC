#include "RabbitMQOrderHandler.hpp"

#include "exchange/logging.hpp"
#include "exchange/rabbitmq/publisher/RabbitMQPublisher.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"

namespace nutc {
namespace rabbitmq {

void
RabbitMQOrderHandler::handle_incoming_market_order(
    engine_manager::EngineManager& engine_manager, manager::ClientManager& clients,
    MarketOrder&& order
)
{
    std::string buffer;
    glz::write<glz::opts{}>(order, buffer);
    std::string replace1 = R"("side":0)";
    std::string replace2 = R"("side":1)";
    size_t pos1 = buffer.find(replace1);
    size_t pos2 = buffer.find(replace2);
    if (pos1 != std::string::npos) {
        buffer.replace(pos1, replace1.length(), R"("side":"buy")");
    }
    if (pos2 != std::string::npos) {
        buffer.replace(pos2, replace2.length(), R"("side":"ask")");
    }

    log_i(rabbitmq, "Received market order: {}", buffer);

    std::optional<std::reference_wrapper<matching::Engine>> engine =
        engine_manager.get_engine(order.ticker);
    assert(engine.has_value()); // TODO: FOR TESTING PURPOSES ONLY
    std::string client_id = order.client_id;
    auto [matches, ob_updates] =
        engine.value().get().match_order(std::move(order), clients);
    for (const auto& match : matches) {
        std::string buyer_id = match.buyer_id;
        std::string seller_id = match.seller_id;
        RabbitMQPublisher::broadcast_account_update(clients, match);
        log_i(
            matching, "Matched order with price {} and quantity {}", match.price,
            match.quantity
        );
    }
    for (const auto& update : ob_updates) {
        log_i(
            rabbitmq, "New ObUpdate with ticker {} price {} quantity {} side {}",
            update.ticker, update.price, update.quantity,
            update.side == messages::SIDE::BUY ? "BUY" : "ASK"
        );
    }
    if (!matches.empty()) {
        RabbitMQPublisher::broadcast_matches(clients, matches);
    }
    if (!ob_updates.empty()) {
        RabbitMQPublisher::broadcast_ob_updates(clients, ob_updates, client_id);
    }
}

} // namespace rabbitmq
} // namespace nutc
