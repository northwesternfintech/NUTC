#pragma once

#include "pywrapper/pywrapper.hpp"
#include "pywrapper/rate_limiter.hpp"
#include "util/messages.hpp"

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

/**
 * @brief The namespace for the NUTC client
 */
namespace nutc {

/**
 * @brief Contains all RabbitMQ related functionality
 */
namespace rabbitmq {
/**
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
    /**
     * @brief Constructor for RabbitMQ (RAII)
     *
     * Initialzies the RMQ connection and creates a queue to receive messages under the
     * given UID
     *
     * @param uid The unique identifier for the client
     */
    RabbitMQ(const std::string& uid);

    /**
     * @brief Destructor for RabbitMQ (RAII)
     *
     * Closes the RMQ connection
     */
    ~RabbitMQ();

    /**
     * @brief Publishes an init message to the exchange
     *
     * Ready indicates whether or not the algorithm successfully initialized (TODO: test
     * init code before) The exchange will run regardless. If the exchange does not
     * receive init messages from all clients, *then* it will not run
     *
     * @param uid The unique identifier for the client_uid
     * @param ready Whether or not the algo is ready to start trading
     *
     * @returns True if the message was successfully published, false otherwise
     */
    bool publishInit(const std::string& uid, bool ready);

    /**
     * @brief Callback for the market order function
     *
     * Used by the wrapper to trigger an order from the py/cpp client
     * Bound to the publishMarketOrder function, but will the client_uid prefilled
     *
     * @param uid The unique identifier for the client
     * @returns A function that takes the order parameters and publishes the order
     */
    std::function<
        bool(const std::string&, const std::string&, float, float)>
    getMarketFunc(const std::string& uid);

    /**
     * @brief Main event loop; handles incoming messages from exchange
     *
     * This is the main event loop of the client. Until given a shutdown message or
     * SIGINT, it will continually receive messages from the exchange (orderbook update,
     * account update, or trade update)
     *
     * @returns A shutdown or error message
     */
    std::variant<ShutdownMessage, RMQError> handleIncomingMessages();

private:
    rate_limiter::RateLimiter limiter;
    bool initializeConnection(const std::string& queueName);
    bool initializeConsume(const std::string& queueName);
    bool connectToRabbitMQ(
        const std::string& hostname,
        int port,
        const std::string& username,
        const std::string& password
    );

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

    std::string consumeMessageAsString();
    std::variant<ShutdownMessage, RMQError, ObUpdate, Match, AccountUpdate>
    consumeMessage();
};

} // namespace rabbitmq
} // namespace nutc
