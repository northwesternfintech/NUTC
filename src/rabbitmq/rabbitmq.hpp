#pragma once

#include "client_manager/manager.hpp"
#include "matching/manager.hpp"
#include "util/messages.hpp"

#include <unistd.h>

#include <iostream>
#include <optional>
#include <string>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

using InitMessage = nutc::messages::InitMessage;
using MarketOrder = nutc::messages::MarketOrder;
using RMQError = nutc::messages::RMQError;
using ShutdownMessage = nutc::messages::ShutdownMessage;
using Match = nutc::messages::Match;
using AccountUpdate = nutc::messages::AccountUpdate;
using Engine = nutc::matching::Engine;

namespace nutc {
namespace rabbitmq {

class RabbitMQ {
public:
    RabbitMQ(manager::ClientManager& manager);
    bool initializeConnection();
    void closeConnection();
    void handleIncomingMessages();
    void addTicker(const std::string& ticker);
    void waitForClients(int num_clients);

private:
    amqp_connection_state_t conn;
    manager::ClientManager& clients;
    engine_manager::Manager engine_manager;
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
    void broadcastMatches(const std::vector<Match>& matches);
    void broadcastObUpdates(const std::vector<ObUpdate>& updates);
    void broadcastAccountUpdate(const Match& match);
    void handleIncomingMarketOrder(const MarketOrder& order);
};

} // namespace rabbitmq
} // namespace nutc
