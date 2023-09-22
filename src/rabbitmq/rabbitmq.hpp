#pragma once

#include "messages.hpp"
#include "pywrapper/pywrapper.hpp"

#include <unistd.h>

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

using InitMessage = nutc::messages::InitMessage;
using MarketOrder = nutc::messages::MarketOrder;
using RMQError = nutc::messages::RMQError;
using ObUpdate = nutc::messages::ObUpdate;
using ShutdownMessage = nutc::messages::ShutdownMessage;
using Match = nutc::messages::Match;

namespace nutc {
namespace rabbitmq {

class RabbitMQ {
public:
    RabbitMQ(const std::string& uid);
    bool initializeConnection(const std::string& queueName);
    bool initializeConsume(const std::string& queueName);
    bool publishInit(const std::string& uid, bool ready);
    std::function<
        bool(const std::string&, const std::string&, const std::string&, float, float)>
    getMarketFunc(const std::string& uid);
    std::variant<ShutdownMessage, RMQError> handleIncomingMessages();
    void closeConnection();

private:
    amqp_connection_state_t conn;
    bool publishMessage(const std::string& queueName, const std::string& message);
    bool initializeQueue(const std::string& queueName);
    bool publishMarketOrder(
        const std::string& client_uid,
        const std::string& side,
        const std::string& type,
        const std::string& ticker,
        float quantity,
        float price
    );

    bool connectToRabbitMQ(
        const std::string& hostname,
        int port,
        const std::string& username,
        const std::string& password
    );

    std::string consumeMessageAsString();
    std::variant<ShutdownMessage, RMQError, ObUpdate, Match> consumeMessage();
};

} // namespace rabbitmq
} // namespace nutc
