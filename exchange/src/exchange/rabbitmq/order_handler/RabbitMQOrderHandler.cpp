#include "RabbitMQOrderHandler.hpp"

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

    if (!engine_manager.has_engine(order.ticker)) {
        return;
    }

    engine_manager.match_order(order);
}

} // namespace rabbitmq
} // namespace nutc
