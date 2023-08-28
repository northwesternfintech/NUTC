#pragma once

#include "glz_templates.hpp"

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

namespace nutc {
namespace rabbitmq {
struct RMQError {
    std::string message;
};

class RabbitMQ {
public:
    bool initializeConnection();
    void closeConnection();
    void wait_for_clients(int num_clients);
    std::variant<InitMessage, MarketOrder, RMQError> consumeMessage();

private:
    amqp_connection_state_t conn;
    std::string consumeMessageAsString();
    bool publishMessage(const std::string& queueName, const std::string& message);
    bool initializeQueue(const std::string& queueName);
    bool initializeConsume(const std::string& queueName);
    bool connectToRabbitMQ(
        const std::string& hostname, int port, const std::string& username,
        const std::string& password
    );
};

} // namespace rabbitmq
} // namespace nutc
