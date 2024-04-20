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
    consume_message_nonblocking();

    static std::variant<messages::init_message, messages::market_order>
    consume_message()
    {
        auto result = consume_message_nonblocking();
        while (!result.has_value()) {
            result = consume_message_nonblocking();
        }
        return result.value();
    }

    static void match_new_order(
        engine_manager::EngineManager& engine_manager, messages::market_order&& order
    );

    /**
     * @brief Main event loop, handles incoming messages from exchange
     *
     * Handles incoming orderbook updates, trade updates, account updates, and shutdown
     * messages from the exchange
     */
    static void consumer_event_loop(engine_manager::EngineManager& engine_manager);

private:
    static std::optional<std::string> consume_message_as_string();
};

} // namespace rabbitmq
} // namespace nutc
