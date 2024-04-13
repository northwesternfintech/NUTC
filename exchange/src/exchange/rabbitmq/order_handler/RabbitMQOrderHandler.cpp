#include "RabbitMQOrderHandler.hpp"

#include "exchange/tick_manager/tick_manager.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_manager.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

namespace nutc {
namespace rabbitmq {

void
RabbitMQOrderHandler::handle_incoming_market_order(
    engine_manager::EngineManager& engine_manager, messages::MarketOrder&& order
)
{
    if (order.price < 0 || order.quantity <= 0)
        return;
    if (!engine_manager.has_engine(order.ticker))
        return;

    auto current_tick = ticks::TickManager::get_instance().get_current_tick();
    auto trader = manager::TraderManager::get_instance().get_trader(order.client_id);
    auto stored_order =
        matching::StoredOrder{trader,         order.side,  order.ticker,
                              order.quantity, order.price, current_tick};

    engine_manager.match_order(stored_order);
}

} // namespace rabbitmq
} // namespace nutc
