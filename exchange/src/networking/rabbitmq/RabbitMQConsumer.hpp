#pragma once

#include "client_manager/client_manager.hpp"
#include "matching/engine_manager.hpp"
#include "logging.hpp"
#include "utils/messages.hpp"

#include <optional>
#include <string>

#include <rabbitmq-c/amqp.h>

namespace nutc {
namespace rabbitmq {

class RabbitMQConsumer {
public:
    static std::variant<
        messages::InitMessage, messages::MarketOrder, messages::RMQError>
    consumeMessage();

    /**
     * @brief Main event loop, handles incoming messages from exchange
     *
     * Handles incoming orderbook updates, trade updates, account updates, and shutdown
     * messages from the exchange
     */
    static void handleIncomingMessages(
        manager::ClientManager& clients, engine_manager::Manager& engine_manager
    );

private:
    static std::optional<std::string> consumeMessageAsString();
};

} // namespace rabbitmq
} // namespace nutc
