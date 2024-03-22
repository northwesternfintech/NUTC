#include "RabbitMQConsumer.hpp"

#include "exchange/concurrency/exchange_lock.hpp"
#include "exchange/logging.hpp"
#include "exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "exchange/rabbitmq/order_handler/RabbitMQOrderHandler.hpp"

#include <utility>

#include <rabbitmq-c/amqp.h>

namespace nutc {
namespace rabbitmq {

void
RabbitMQConsumer::handle_incoming_messages(engine_manager::EngineManager& engine_manager
)
{
    while (true) {
        concurrency::ExchangeLock::get_instance().lock();
        auto incoming_message = consume_message(10);
        if (!incoming_message.has_value()) {
            concurrency::ExchangeLock::get_instance().unlock();
            continue;
        }

        // Use std::visit to deal with the variant
        std::visit(
            [&](auto&& arg) {
                using t = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<t, messages::InitMessage>) {
                    log_c(rabbitmq, "Not expecting initialization message");
                    std::abort();
                }
                else if constexpr (std::is_same_v<t, messages::MarketOrder>) {
                    RabbitMQOrderHandler::handle_incoming_market_order(
                        engine_manager, std::forward<t>(arg)
                    );
                }
                else {
                    log_e(rabbitmq, "Unknown message type");
                    std::abort();
                }
            },
            std::move(incoming_message.value())
        );
        concurrency::ExchangeLock::get_instance().unlock();
    }
}

std::optional<std::string>
RabbitMQConsumer::consume_message_as_string(int timeout_us)
{
    const auto& connection_state =
        RabbitMQConnectionManager::get_instance().get_connection_state();

    amqp_envelope_t envelope;
    amqp_maybe_release_buffers(connection_state);

    struct timeval timeout {};

    timeout.tv_usec = timeout_us;
    amqp_rpc_reply_t res;
    if (timeout_us == 0) {
        res = amqp_consume_message(connection_state, &envelope, nullptr, 0);
    }
    else {
        res = amqp_consume_message(connection_state, &envelope, &timeout, 0);
    }

    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        switch (res.reply_type) {
            case AMQP_RESPONSE_LIBRARY_EXCEPTION:
                if (amqp_error_string2(res.library_error)
                    == std::string("request timed out"))
                    break;
                log_e(
                    rabbitmq, "Library exception: {}",
                    amqp_error_string2(res.library_error)
                );
                break;
            case AMQP_RESPONSE_SERVER_EXCEPTION:
                log_e(rabbitmq, "Server exception: {}", res.reply.id);
                break;
            case AMQP_RESPONSE_NONE:
                log_d(rabbitmq, "No incoming messages");
                break;
            default:
                log_e(rabbitmq, "Unknown error.");
                break;
        }
        return std::nullopt;
    }

    std::string message(
        static_cast<char*>(envelope.message.body.bytes), envelope.message.body.len
    );
    amqp_destroy_envelope(&envelope);
    return message;
}

std::optional<std::variant<messages::InitMessage, messages::MarketOrder>>
RabbitMQConsumer::consume_message(int timeout_us)
{
    std::optional<std::string> buf = consume_message_as_string(timeout_us);
    if (!buf.has_value()) {
        return std::nullopt;
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
