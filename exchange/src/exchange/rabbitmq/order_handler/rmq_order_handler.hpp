#pragma once

#include "exchange/tickers/manager/ticker_manager.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

namespace nutc {
namespace rabbitmq {
class RabbitMQOrderHandler {
public:
    static void handle_incoming_market_order(
        engine_manager::EngineManager& engine_manager, messages::market_order&& order
    );
};

} // namespace rabbitmq
} // namespace nutc
