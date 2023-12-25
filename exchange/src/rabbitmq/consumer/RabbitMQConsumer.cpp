#include "RabbitMQConsumer.hpp"

#include "logging.hpp"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "rabbitmq/order_handler/RabbitMQOrderHandler.hpp"

#include <rabbitmq-c/amqp.h>

namespace nutc {
namespace rabbitmq {

void
RabbitMQConsumer::handleIncomingMessages(
    manager::ClientManager& clients, engine_manager::Manager& engine_manager
)
{
    while (true) {
        auto incoming_message = consumeMessage();

        // Use std::visit to deal with the variant
        std::visit(
            [&](auto&& arg) {
                using t = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<t, messages::InitMessage>) {
                    log_e(rabbitmq, "Not expecting initialization message");
                    std::abort();
                }
                else if constexpr (std::is_same_v<t, messages::MarketOrder>) {
                    RabbitMQOrderHandler::handleIncomingMarketOrder(
                        engine_manager, clients, arg
                    );
                }
            },
            incoming_message
        );
    }
}

std::optional<std::string>
RabbitMQConsumer::consumeMessageAsString()
{
    const auto& connection_state =
        RabbitMQConnectionManager::get_instance().get_connection_state();

    amqp_envelope_t envelope;
    amqp_maybe_release_buffers(connection_state);
    amqp_rpc_reply_t res =
        amqp_consume_message(connection_state, &envelope, nullptr, 0);

    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to consume message.");
        return std::nullopt;
    }

    std::string message(
        static_cast<char*>(envelope.message.body.bytes), envelope.message.body.len
    );
    amqp_destroy_envelope(&envelope);
    return message;
}

std::variant<messages::InitMessage, messages::MarketOrder>
RabbitMQConsumer::consumeMessage()
{
    std::optional<std::string> buf = consumeMessageAsString();
    if (!buf.has_value()) {
        // todo: throw exception instead
        std::abort();
    }

    std::variant<messages::InitMessage, messages::MarketOrder> data;
    auto err = glz::read_json(data, buf.value());
    if (err) {
        std::abort();
    }
    return data;
}

} // namespace rabbitmq
} // namespace nutc
