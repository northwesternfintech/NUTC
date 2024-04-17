#include "rmq_connection_handler.hpp"

#include "exchange/logging.hpp"
#include "exchange/rabbitmq/queue_manager/RabbitMQQueueManager.hpp"

#include <glaze/glaze.hpp>

#include <rabbitmq-c/tcp_socket.h>

namespace nutc {
namespace rabbitmq {

bool
RabbitMQConnectionManager::connect_to_rabbitmq_(
    const std::string& hostname, int port, const std::string& username,
    const std::string& password
)
{
    connection_state_ = amqp_new_connection();
    amqp_socket_t* socket = amqp_tcp_socket_new(connection_state_);

    if (socket == nullptr) {
        log_e(rabbitmq, "{}", "Failed to create TCP socket.");
    }
    auto status =
        static_cast<amqp_status_enum>(amqp_socket_open(socket, hostname.c_str(), port));
    if (status != AMQP_STATUS_OK) {
        log_e(rabbitmq, "Failed to open TCP socket: {}", amqp_error_string2(status));
        return false;
    }

    constexpr int FRAME_MAX = 131072;

    amqp_rpc_reply_t reply = amqp_login( // NOLINT (cppcoreguidelines-pro-type-vararg)
        connection_state_, "/", 0, FRAME_MAX, 0, AMQP_SASL_METHOD_PLAIN,
        username.c_str(), password.c_str()
    );

    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "{}", "Failed to login to RabbitMQ.");
        return false;
    }

    return true;
}

void
RabbitMQConnectionManager::close_connection()
{
    if (connection_state_ == nullptr)
        return;
    // Close channel and connection, then destroy connection
    amqp_channel_close(connection_state_, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(connection_state_, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(connection_state_);
}

bool
RabbitMQConnectionManager::initialize_connection_()
{
    if (!connect_to_rabbitmq_("localhost", RABBITMQ_PORT, "NUFT", "ADMIN"))
        return false;

    amqp_channel_open(connection_state_, 1);
    amqp_rpc_reply_t res = amqp_get_rpc_reply(connection_state_);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to open channel.");
        return false;
    }

    if (!RabbitMQQueueManager::initialize_queue(connection_state_, "market_order")) {
        log_e(rabbitmq, "Failed to initialize queue.");
        return false;
    }
    if (!RabbitMQQueueManager::initialize_exchange(
            connection_state_, "fanout_to_wrappers"
        )) {
        log_e(rabbitmq, "Failed to initialize exchange.");
        return false;
    }
    if (!RabbitMQQueueManager::initialize_consume(connection_state_, "market_order")) {
        log_e(rabbitmq, "Failed to initialize consume.");
        return false;
    }

    return true;
}

} // namespace rabbitmq
} // namespace nutc
