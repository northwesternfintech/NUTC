#include "RabbitMQQueueManager.hpp"

#include "exchange/logging.hpp"

namespace nutc {
namespace rabbitmq {

bool
RabbitMQQueueManager::initialize_queue(
    const amqp_connection_state_t& connection_state, const std::string& queue_name
)
{
    amqp_queue_declare(
        connection_state, 1, amqp_cstring_bytes(queue_name.c_str()), 0, 0, 0, 1,
        amqp_empty_table
    );
    amqp_rpc_reply_t res = amqp_get_rpc_reply(connection_state);

    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to declare queue.");
        return false;
    }
    log_i(rabbitmq, "Declared queue: {}", queue_name);

    return true;
}

bool
RabbitMQQueueManager::initialize_consume(
    const amqp_connection_state_t& connection_state, const std::string& queue_name
)
{
    amqp_basic_consume(
        connection_state, 1, amqp_cstring_bytes(queue_name.c_str()), amqp_empty_bytes,
        0, 1, 0, amqp_empty_table
    );

    amqp_rpc_reply_t res = amqp_get_rpc_reply(connection_state);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to consume message.");
        return false;
    }

    return true;
}

} // namespace rabbitmq
} // namespace nutc
