#include "networking/rabbitmq/RabbitMQQueueManager.hpp"

#include "logging.hpp"
#include "networking/rabbitmq/RabbitMQConnectionManager.hpp"

#include <rabbitmq-c/amqp.h>

namespace nutc {
namespace rabbitmq {

bool
RabbitMQQueueManager::initializeQueue(
    const amqp_connection_state_t& connection_state, const std::string& queueName
)
{
    amqp_queue_declare(
        connection_state, 1, amqp_cstring_bytes(queueName.c_str()), 0, 0, 0, 1,
        amqp_empty_table
    );
    amqp_rpc_reply_t res = amqp_get_rpc_reply(connection_state);

    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to declare queue.");
        return false;
    }
    log_i(rabbitmq, "Declared queue: {}", queueName);

    return true;
}

bool
RabbitMQQueueManager::initializeConsume(
    const amqp_connection_state_t& connection_state, const std::string& queueName
)
{
    amqp_basic_consume(
        connection_state, 1, amqp_cstring_bytes(queueName.c_str()), amqp_empty_bytes, 0,
        1, 0, amqp_empty_table
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
