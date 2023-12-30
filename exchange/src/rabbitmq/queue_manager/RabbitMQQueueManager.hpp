#pragma once

#include <string>

#include <rabbitmq-c/amqp.h>

namespace nutc {
namespace rabbitmq {
class RabbitMQQueueManager {
public:
    static bool initialize_consume(
        const amqp_connection_state_t& connection_state, const std::string& queue_name
    );
    static bool initialize_queue(
        const amqp_connection_state_t& connection_state, const std::string& queue_name
    );
};

} // namespace rabbitmq
} // namespace nutc
