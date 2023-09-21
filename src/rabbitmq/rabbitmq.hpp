#pragma once

#include "client_manager/manager.hpp"
#include "matching/engine.hpp"
#include "messages.hpp"

#include <unistd.h>

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

using InitMessage = nutc::messages::InitMessage;
using MarketOrder = nutc::messages::MarketOrder;
using RMQError = nutc::messages::RMQError;
using ShutdownMessage = nutc::messages::ShutdownMessage;
using Match = nutc::matching::Match;

namespace nutc {
namespace rabbitmq {

class RabbitMQ {
public:
    bool initializeConnection();
    void closeConnection(const nutc::manager::ClientManager& users);
    void handleIncomingMessages(nutc::matching::Engine& engine);
    void waitForClients(int num_clients, nutc::manager::ClientManager& users);

private:
    amqp_connection_state_t conn;
    bool logAndReturnError(const char* errorMessage);
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
