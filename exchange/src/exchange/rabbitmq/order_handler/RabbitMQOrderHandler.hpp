#pragma once

#include "exchange/client_manager/client_manager.hpp"
#include "exchange/matching/manager/engine_manager.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <string>

namespace nutc {
namespace rabbitmq {
class RabbitMQOrderHandler {
public:
    static void add_liquidity_to_ticker(
        manager::ClientManager& clients, engine_manager::EngineManager& engine_manager,
        const std::string& ticker, float quantity, float price
    );
    static void handle_incoming_market_order(
        engine_manager::EngineManager& engine_manager, manager::ClientManager& clients,
        messages::MarketOrder&& order
    );
};

} // namespace rabbitmq
} // namespace nutc
