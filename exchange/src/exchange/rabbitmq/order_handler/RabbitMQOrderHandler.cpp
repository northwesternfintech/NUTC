#include "RabbitMQOrderHandler.hpp"

#include "exchange/tickers/manager/ticker_manager.hpp"

namespace nutc {
namespace rabbitmq {

void
RabbitMQOrderHandler::handle_incoming_market_order(
    engine_manager::EngineManager& engine_manager, MarketOrder&& order
)
{
    if (order.price < 0 || order.quantity <= 0)
        return;
    if (!engine_manager.has_engine(order.ticker))
        return;

    engine_manager.match_order(order);
}

} // namespace rabbitmq
} // namespace nutc
