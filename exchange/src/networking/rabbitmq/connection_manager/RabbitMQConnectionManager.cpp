#include "RabbitMQConnectionManager.hpp"

#include "logging.hpp"
#include "networking/rabbitmq/publisher/RabbitMQPublisher.hpp"
#include "networking/rabbitmq/queue_manager/RabbitMQQueueManager.hpp"
#include "utils/messages.hpp"

#include <glaze/glaze.hpp>

#include <rabbitmq-c/tcp_socket.h>

namespace nutc {
namespace rabbitmq {

bool
RabbitMQConnectionManager::connectToRabbitMQ(
    const std::string& hostname, int port, const std::string& username,
    const std::string& password
)
{
    amqp_socket_t* socket = amqp_tcp_socket_new(connection_state);

    if (!socket) {
        log_e(rabbitmq, "{}", "Failed to create TCP socket.");
    }
    if (amqp_socket_open(socket, hostname.c_str(), port)) {
        log_e(rabbitmq, "{}", "Failed to open TCP socket.");
        return false;
    }

    amqp_rpc_reply_t reply = amqp_login(
        connection_state, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, username.c_str(),
        password.c_str()
    );
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "{}", "Failed to login to RabbitMQ.");
        return false;
    }

    return true;
}

void
RabbitMQConnectionManager::closeConnection(const manager::ClientManager& client_manager)
{
    // Handle client shutdown
    auto shutdownClient = [&](const auto& client) {
        log_i(rabbitmq, "Shutting down client {}", client.uid);
        messages::ShutdownMessage shutdown{client.uid};
        auto messageStr = glz::write_json(shutdown);
        RabbitMQPublisher::publishMessage(client.uid, messageStr);
    };

    // Iterate over clients and shut them down
    for (const auto& client : client_manager.get_clients(true)) {
        shutdownClient(client);
    }

    // Close channel and connection, then destroy connection
    amqp_channel_close(connection_state, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(connection_state, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(connection_state);
}

bool
RabbitMQConnectionManager::connectedToRMQ()
{
    return connected;
}

amqp_connection_state_t
RabbitMQConnectionManager::get_connection_state()
{
    return connection_state;
}

bool
RabbitMQConnectionManager::initializeConnection()
{
    if (!connectToRabbitMQ("localhost", 5672, "NUFT", "ADMIN"))
        return false;

    amqp_channel_open(connection_state, 1);
    amqp_rpc_reply_t res = amqp_get_rpc_reply(connection_state);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to open channel.");
        return false;
    }

    if (!RabbitMQQueueManager::initializeQueue(connection_state, "market_order")) {
        log_e(rabbitmq, "Failed to initialize queue.");
        return false;
    }
    if (!RabbitMQQueueManager::initializeConsume(connection_state, "market_order")) {
        log_e(rabbitmq, "Failed to initialize consume.");
        return false;
    }

    return true;
}

} // namespace rabbitmq
} // namespace nutc
