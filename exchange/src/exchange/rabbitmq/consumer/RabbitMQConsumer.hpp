#pragma once

#include "exchange/tickers/manager/ticker_manager.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <optional>
#include <string>

namespace nutc {
namespace rabbitmq {

class RabbitMQConsumer {
public:
    static std::optional<std::variant<messages::init_message, messages::market_order>>
    consume_message(int timeout_us);

    static std::variant<messages::init_message, messages::market_order>
    consume_message()
    {
        auto message = consume_message(0);
        assert(message.has_value());
        return message.value();
    }

    /**
     * @brief Main event loop, handles incoming messages from exchange
     *
     * Handles incoming orderbook updates, trade updates, account updates, and shutdown
     * messages from the exchange
     */
    static void handle_incoming_messages(engine_manager::EngineManager& engine_manager);

private:
    static std::optional<std::string> consume_message_as_string(int timeout_us);
};

} // namespace rabbitmq
} // namespace nutc
