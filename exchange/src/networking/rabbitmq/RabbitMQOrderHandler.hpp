#pragma once

#include "client_manager/client_manager.hpp"
#include "matching/engine_manager.hpp"
#include "utils/messages.hpp"

#include <string>

namespace nutc {
namespace rabbitmq {
class RabbitMQOrderHandler {
public:
    static void addLiquidityToTicker(
        manager::ClientManager& clients, engine_manager::Manager& engine_manager,
        const std::string& ticker, float quantity, float price
    );
    static void handleIncomingMarketOrder(
        engine_manager::Manager& engine_manager, manager::ClientManager& clients,
        messages::MarketOrder& order
    );
};

} // namespace rabbitmq
} // namespace nutc
