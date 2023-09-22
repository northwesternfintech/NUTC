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
using AccountUpdate = nutc::messages::AccountUpdate;

namespace nutc {
namespace rabbitmq {
/*
 * @class RabbitMQ
 * @brief Handles all RabbitMQ related functionality; main event loop
 *
 * Main event loop (i.e., program loops on this class)
 * Handles initialization and closure of the RMQ connection
 * Handles incoming messages from exchange (i.e., order book updates, matches, etc.)
 * Handles outgoing messages to exchange (i.e., market orders)
 * Provides a callback for the market order function (so it can be triggered by algo)
 * Calls the algo's callbacks when receiving a message from the exchange (order book
 * update, account update, etc)
 */
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
    std::variant<ShutdownMessage, RMQError, ObUpdate, Match, AccountUpdate>
    consumeMessage();
};

} // namespace rabbitmq
} // namespace nutc
