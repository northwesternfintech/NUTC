#pragma once

#include "exchange/client_manager/client_manager.hpp"
#include "exchange/matching/manager/engine_manager.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <optional>
#include <string>

namespace nutc {
namespace rabbitmq {

class RabbitMQConsumer {
public:
    static std::variant<messages::InitMessage, messages::MarketOrder> consume_message();

    /**
     * @brief Main event loop, handles incoming messages from exchange
     *
     * Handles incoming orderbook updates, trade updates, account updates, and shutdown
     * messages from the exchange
     */
    static void handle_incoming_messages(
        manager::ClientManager& clients, engine_manager::EngineManager& engine_manager
    );

private:
    static std::optional<std::string> consume_message_as_string();
};

} // namespace rabbitmq
} // namespace nutc
