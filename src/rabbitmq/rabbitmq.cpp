#include "rabbitmq.hpp"

#include "logging.hpp"

namespace nutc {
namespace rabbitmq {
bool
RabbitMQ::connectToRabbitMQ(
    const std::string& hostname,
    int port,
    const std::string& username,
    const std::string& password
)
{
    conn = amqp_new_connection();
    amqp_socket_t* socket = amqp_tcp_socket_new(conn);

    if (!socket) {
        log_e(rabbitmq, "Cannot create TCP socket");
        return false;
    }

    int status = amqp_socket_open(socket, hostname.c_str(), port);
    if (status) {
        log_e(rabbitmq, "Cannot open socket");
        return false;
    }

    amqp_rpc_reply_t reply = amqp_login(
        conn,
        "/",
        0,
        131072,
        0,
        AMQP_SASL_METHOD_PLAIN,
        username.c_str(),
        password.c_str()
    );
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Login failed");
        return false;
    }

    return true;
}

std::variant<ShutdownMessage, RMQError>
RabbitMQ::handleIncomingMessages()
{
    while (true) {
        std::variant<ShutdownMessage, RMQError, ObUpdate> data = consumeMessage();
        if (std::holds_alternative<ShutdownMessage>(data)) {
            log_w(
                rabbitmq,
                "Received shutdown message: {}",
                std::get<ShutdownMessage>(data).shutdown_reason
            );
            return std::get<ShutdownMessage>(data);
        }
        else if (std::holds_alternative<RMQError>(data)) {
            log_e(
                rabbitmq,
                "Failed to consume message: {}",
                std::get<RMQError>(data).message
            );
            return std::get<RMQError>(data);
        }
        else if (std::holds_alternative<ObUpdate>(data)) {
            log_i(
                rabbitmq,
                "Received order book update: {}",
                glz::write_json(std::get<ObUpdate>(data))
            );
        }
        else {
            log_e(rabbitmq, "Unknown message type");
            return RMQError{"Unknown message type"};
        }
    }
}

bool
RabbitMQ::publishMarketOrder(
    const std::string& security,
    float quantity,
    bool side,
    const std::string& type,
    float price
)
{
    MarketOrder order{security, quantity, side, type, price};
    order.security = security;
    order.quantity = quantity;
    order.side = side;
    order.type = type;
    std::string message = glz::write_json(order);

    log_i(rabbitmq, "Publishing order: {}", message);
    return publishMessage("market_order", message);
}

bool
RabbitMQ::publishMessage(const std::string& queueName, const std::string& message)
{
    amqp_basic_publish(
        conn,
        1,
        amqp_cstring_bytes(""),
        amqp_cstring_bytes(queueName.c_str()),
        0,
        0,
        NULL,
        amqp_cstring_bytes(message.c_str())
    );

    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to publish message.");
        return false;
    }

    return true;
}

std::variant<ShutdownMessage, RMQError, ObUpdate>
RabbitMQ::consumeMessage()
{
    std::string buf = consumeMessageAsString();
    if (buf == "") {
        return RMQError{"Failed to consume message."};
    }
    log_i(rabbitmq, "{}", buf);

    std::variant<ShutdownMessage, RMQError, ObUpdate> data{};
    auto err = glz::read_json(data, buf);
    if (err) {
        std::string error = glz::format_error(err, buf);
        return RMQError{error};
    }
    return data;
}

// Blocking
std::string
RabbitMQ::consumeMessageAsString()
{
    amqp_envelope_t envelope;
    amqp_maybe_release_buffers(conn);
    amqp_rpc_reply_t res = amqp_consume_message(conn, &envelope, NULL, 0);

    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to consume message.");
        return "";
    }

    std::string message(
        reinterpret_cast<char*>(envelope.message.body.bytes), envelope.message.body.len
    );
    amqp_destroy_envelope(&envelope);
    return message;
}

bool
RabbitMQ::initializeConnection(const std::string& queueName)
{
    if (!connectToRabbitMQ("localhost", 5672, "NUFT", "ADMIN")) {
        log_c(rabbitmq, "Failed to connect to RabbitMQ");
        return false;
    }
    amqp_channel_open(conn, 1);
    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to open channel.");
        return false;
    }

    if (!initializeQueue(queueName)) {
        return false;
    }

    if (!initializeConsume(queueName)) {
        return false;
    }

    log_i(rabbitmq, "Connection established");

    return true;
}

bool
RabbitMQ::initializeConsume(const std::string& queueName)
{
    amqp_basic_consume(
        conn,
        1,
        amqp_cstring_bytes(queueName.c_str()),
        amqp_empty_bytes,
        0,
        1,
        0,
        amqp_empty_table
    );

    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to consume message.");
        return false;
    }

    return true;
}

RabbitMQ::RabbitMQ(const std::string& uid)
{
    if (!initializeConnection(uid)) {
        log_c(rabbitmq, "Failed to initialize connection to RabbitMQ");
        // attempt to say we didn't init correctly
        publishInit(uid, false);
        exit(1);
    }
    publishInit(uid, true);
}

std::function<bool(const std::string&, float, bool, const std::string&, float)>
RabbitMQ::getMarketFunc()
{
    return std::bind(
        &RabbitMQ::publishMarketOrder,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4,
        std::placeholders::_5
    );
}

bool
RabbitMQ::publishInit(const std::string& uid, bool ready)
{
    std::string message = glz::write_json(InitMessage{uid, ready});
    log_i(rabbitmq, "Publishing init message: {}", message);
    return publishMessage("market_order", message);
}

bool
RabbitMQ::initializeQueue(const std::string& queueName)
{
    amqp_queue_declare(
        conn, 1, amqp_cstring_bytes(queueName.c_str()), 0, 0, 0, 1, amqp_empty_table
    );

    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to declare queue.");
        return false;
    }
    log_d(rabbitmq, "Declared queue: {}", queueName);

    return true;
}

void
RabbitMQ::closeConnection()
{
    amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);
}

} // namespace rabbitmq
} // namespace nutc
