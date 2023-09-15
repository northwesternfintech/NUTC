#pragma once

#include "client_manager/manager.hpp"
#include "glz_templates.hpp"
#include "matching/engine.hpp"

#include <unistd.h>

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

using InitMessage = nutc::messages::InitMessage;
using MarketOrder = nutc::messages::MarketOrder;
using RMQError = nutc::messages::RMQError;
using ShutdownMessage = nutc::messages::ShutdownMessage;

namespace nutc {
namespace rabbitmq {

class RabbitMQ {
public:
    bool initializeConnection();
    void closeConnection(const nutc::manager::ClientManager& users);
    void handle_incoming_messages(nutc::matching::Engine& engine);
    void wait_for_clients(int num_clients, nutc::manager::ClientManager& users);

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
