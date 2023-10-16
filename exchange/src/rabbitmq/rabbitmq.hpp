#pragma once

#include "client_manager/client_manager.hpp"
#include "matching/engine_manager.hpp"
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

/**
 * @brief Handles all RabbitMQ communication for the exchange
 */
namespace rabbitmq {

/**
 * @class RabbitMQ
 * @brief Handles all RabbitMQ communication for the exchange
 */
class RabbitMQ {
public:
    /**
     * @brief Initializes a RabbitMQ connection with a given ClientManager (RAII)
     *
     * @param manager The ClientManager to use for client management
     */
    RabbitMQ(manager::ClientManager& manager, engine_manager::Manager& matching_manager);

    /**
     * @brief Disconnects from RabbitMQ (RAII)
     */
    ~RabbitMQ();

    /**
     * @brief Main event loop, handles incoming messages from exchange
     *
     * Handles incoming orderbook updates, trade updates, account updates, and shutdown
     * messages from the exchange
     */
    void handleIncomingMessages();

    /**
     * @brief Adds a ticker to the encapsulated engine manager
     * */
    void addTicker(const std::string& ticker);
    void addLiquidityToTicker(const std::string& ticker, float quantity, float price);

    /**
     * @brief On startup, waits for all clients to send an initialization message
     *
     * This ensures that all clients are connected to the exchange and have successfully
     * started (vs a RMQ or firebase error)
     */
    void waitForClients(int num_clients);

    /**
     * @brief Returns whether the class is connected to rabbitmq
     */
    bool connectedToRMQ();

    void sendInitialLiquidity();

private:
    amqp_connection_state_t conn;
    manager::ClientManager& clients;
    engine_manager::Manager& engine_manager;
    bool connected;
    bool logAndReturnError(const char* errorMessage);
    bool initializeConnection();
    void closeConnection();
    std::optional<std::string> consumeMessageAsString();
    bool publishMessage(const std::string& queueName, const std::string& message);
    std::variant<InitMessage, MarketOrder, RMQError> consumeMessage();
    bool initializeQueue(const std::string& queueName);
    bool initializeConsume(const std::string& queueName);
    bool connectToRabbitMQ(
        const std::string& hostname, int port, const std::string& username,
        const std::string& password
    );
    void broadcastMatches(const std::vector<Match>& matches);
    void broadcastObUpdates(
        const std::vector<ObUpdate>& updates, const std::string& ignore_uid
    );
    void broadcastAccountUpdate(const Match& match);
    void handleIncomingMarketOrder(MarketOrder& order);
};

} // namespace rabbitmq
} // namespace nutc
