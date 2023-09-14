#pragma once

#include "glz_templates.hpp"
#include "matching/engine.hpp"

#include <iostream>
#include <string>
#include <unistd.h>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

namespace nutc {
namespace rabbitmq {

class RabbitMQ {
public:
    bool initializeConnection();
    void closeConnection(glz::json_t::object_t users);
    void handle_incoming_messages(nutc::matching::Engine engine);
    void wait_for_clients(int num_clients);

private:
    amqp_connection_state_t conn;
    std::string consumeMessageAsString();
    bool publishMessage(const std::string& queueName, const std::string& message);
    std::variant<InitMessage, MarketOrder, RMQError> consumeMessage();
    bool initializeQueue(const std::string& queueName);
    bool initializeConsume(const std::string& queueName);
    bool connectToRabbitMQ(
        const std::string& hostname, int port, const std::string& username,
        const std::string& password
    );
};

} // namespace rabbitmq
} // namespace nutc
