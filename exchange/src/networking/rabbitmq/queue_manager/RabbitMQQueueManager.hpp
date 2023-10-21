#pragma once

#include <string>

#include <rabbitmq-c/amqp.h>

namespace nutc {
namespace rabbitmq {
class RabbitMQQueueManager {
public:
    static bool initializeConsume(
        const amqp_connection_state_t& connection_state, const std::string& queueName
    );
    static bool initializeQueue(
        const amqp_connection_state_t& connection_state, const std::string& queueName
    );
};

} // namespace rabbitmq
} // namespace nutc
