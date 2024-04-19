#pragma once

#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "wrapper/pywrapper/rate_limiter.hpp"

#include <unistd.h>

#include <string>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

using init_message = nutc::messages::init_message;
using market_order = nutc::messages::market_order;
using orderbook_update = nutc::messages::orderbook_update;
using Match = nutc::messages::match;
using start_time = nutc::messages::start_time;

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
    amqp_connection_state_t conn{};

public:
    /**
     * @brief Constructor for RabbitMQ (RAII)
     *
     * Initialzies the RMQ connection and creates a queue to receive messages under the
     * given UID
     *
     * @param uid The unique identifier for the client
     */
    explicit RabbitMQ(const std::string& user_id);

    bool bind_queue_to_exchange(
        const std::string& queue_name, const std::string& exchange_name
    );

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
    [[nodiscard]] bool publishInit(const std::string& user_id, bool ready);

    /**
     * @brief Callback for the market order function
     *
     * Used by the wrapper to trigger an order from the py/cpp client
     * Bound to the publishmarket_order function, but will the client_uid prefilled
     *
     * @param uid The unique identifier for the client
     * @returns A function that takes the order parameters and publishes the order
     */
    std::function<bool(const std::string&, const std::string&, double, double)>
    getMarketFunc(const std::string& user_id);

    void wait_for_start_time(bool skip_start_wait);

    /**
     * @brief Main event loop; handles incoming messages from exchange
     *
     * This is the main event loop of the client. Until given a shutdown message or
     * SIGINT, it will continually receive messages from the exchange (orderbook update,
     * account update, or trade update)
     *
     * @returns A shutdown or error message
     */
    void handleIncomingMessages(const std::string& uid);

private:
    rate_limiter::RateLimiter limiter;
    [[nodiscard]] bool initializeConnection(const std::string& queue_name);
    [[nodiscard]] bool initializeConsume(const std::string& queueName);
    [[nodiscard]] bool connectToRabbitMQ(
        const std::string& hostname, int port, const std::string& username,
        const std::string& password
    );

    [[nodiscard]] bool
    publishMessage(const std::string& queueName, const std::string& message);
    [[nodiscard]] bool initializeQueue(const std::string& queueName);
    [[nodiscard]] bool publishmarket_order(
        const std::string& client_id, const std::string& side,
        const std::string& ticker, double quantity, double price
    );

    std::string consumeMessageAsString();
    std::variant<start_time, orderbook_update, Match> consumeMessage();
};

} // namespace rabbitmq
} // namespace nutc
