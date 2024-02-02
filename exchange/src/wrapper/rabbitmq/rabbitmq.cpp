#include "rabbitmq.hpp"

#include "wrapper/logging.hpp"

#include <chrono>

namespace nutc {
namespace rabbitmq {

bool
RabbitMQ::connectToRabbitMQ(
    const std::string& hostname, int port, const std::string& username,
    const std::string& password
)
{
    conn = amqp_new_connection();
    amqp_socket_t* socket = amqp_tcp_socket_new(conn);

    if (!socket) {
        log_e(wrapper_rabbitmq, "Cannot create TCP socket");
        return false;
    }

    int status = amqp_socket_open(socket, hostname.c_str(), port);
    if (status) {
        log_e(wrapper_rabbitmq, "Cannot open socket");
        return false;
    }

    amqp_rpc_reply_t reply = amqp_login(
        conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, username.c_str(),
        password.c_str()
    );
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(wrapper_rabbitmq, "Login failed");
        return false;
    }

    return true;
}

// todo: split into helpers
void
RabbitMQ::handleIncomingMessages()
{
    while (true) {
        std::variant<StartTime, ObUpdate, Match, AccountUpdate> data = consumeMessage();
        std::visit(
            [&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, ObUpdate>) {
                    log_i(
                        wrapper_rabbitmq, "Received order book update: {}",
                        glz::write_json(std::get<ObUpdate>(data))
                    );
                    ObUpdate update = std::get<ObUpdate>(data);
                    std::string side =
                        update.side == messages::SIDE::BUY ? "BUY" : "SELL";
                    nutc::pywrapper::get_ob_update_function()(
                        update.ticker, side, update.price, update.quantity
                    );
                    return;
                }
                else if constexpr (std::is_same_v<T, Match>) {
                    log_i(
                        wrapper_rabbitmq, "Received match: {}",
                        glz::write_json(std::get<Match>(data))
                    );
                    Match match = std::get<Match>(data);
                    std::string side =
                        match.side == messages::SIDE::BUY ? "BUY" : "SELL";
                    nutc::pywrapper::get_trade_update_function()(
                        match.ticker, side, match.price, match.quantity
                    );
                    return;
                }
                else if constexpr (std::is_same_v<T, AccountUpdate>) {
                    AccountUpdate update = std::get<AccountUpdate>(data);
                    log_i(
                        wrapper_rabbitmq,
                        "Received account update with capital remaining: {}",
                        update.capital_remaining
                    );
                    std::string side =
                        update.side == messages::SIDE::BUY ? "BUY" : "SELL";
                    nutc::pywrapper::get_account_update_function()(
                        update.ticker, side, update.price, update.quantity,
                        update.capital_remaining
                    );
                    return;
                }
                else {
                    return;
                }
            },
            data
        );
    }
}

bool
RabbitMQ::publishMarketOrder(
    const std::string& client_id, const std::string& side, const std::string& ticker,
    float quantity, float price
)
{
    if (limiter.should_rate_limit()) {
        return false;
    }
    MarketOrder order{
        client_id, side == "BUY" ? messages::SIDE::BUY : messages::SIDE::SELL, ticker,
        quantity, price
    };
    std::string message = glz::write_json(order);

    log_i(wrapper_rabbitmq, "Publishing order: {}", message);
    return publishMessage("market_order", message);
}

bool
RabbitMQ::publishMessage(const std::string& queueName, const std::string& message)
{
    amqp_basic_publish(
        conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes(queueName.c_str()), 0, 0,
        NULL, amqp_cstring_bytes(message.c_str())
    );

    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(wrapper_rabbitmq, "Failed to publish message.");
        return false;
    }

    return true;
}

std::variant<StartTime, ObUpdate, Match, AccountUpdate>
RabbitMQ::consumeMessage()
{
    std::string buf = consumeMessageAsString();
    if (buf.empty()) {
        log_e(wrapper_rabbitmq, "Failed to consume message.");
        exit(1);
    }

    std::variant<StartTime, ObUpdate, Match, AccountUpdate> data{};
    auto err = glz::read_json(data, buf);
    if (err) {
        std::string error = glz::format_error(err, buf);
        log_e(wrapper_rabbitmq, "Failed to parse message: {}", error);
        exit(1);
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
        log_e(wrapper_rabbitmq, "Failed to consume message.");
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
        log_c(wrapper_rabbitmq, "Failed to connect to RabbitMQ");
        return false;
    }
    amqp_channel_open(conn, 1);
    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(wrapper_rabbitmq, "Failed to open channel.");
        return false;
    }

    if (!initializeQueue(queueName)) {
        return false;
    }

    if (!initializeConsume(queueName)) {
        return false;
    }

    log_i(wrapper_rabbitmq, "Connection established");

    return true;
}

bool
RabbitMQ::initializeConsume(const std::string& queueName)
{
    amqp_basic_consume(
        conn, 1, amqp_cstring_bytes(queueName.c_str()), amqp_empty_bytes, 0, 1, 0,
        amqp_empty_table
    );

    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(wrapper_rabbitmq, "Failed to consume message.");
        return false;
    }

    return true;
}

RabbitMQ::RabbitMQ(const std::string& id)
{
    if (!initializeConnection(id)) {
        log_c(wrapper_rabbitmq, "Failed to initialize connection to RabbitMQ");
        // attempt to say we didn't init correctly
        bool published_init = publishInit(id, false);
        if (!published_init) {
            log_e(wrapper_rabbitmq, "Failed to publish init message");
        }

        exit(1);
    }
}

std::function<bool(const std::string&, const std::string&, float, float)>
RabbitMQ::getMarketFunc(const std::string& id)
{
    return std::bind(
        &RabbitMQ::publishMarketOrder, this, id, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4
    );
}

bool
RabbitMQ::publishInit(const std::string& id, bool ready)
{
    std::string message = glz::write_json(InitMessage{id, ready});
    log_i(wrapper_rabbitmq, "Publishing init message: {}", message);
    bool rVal = publishMessage("market_order", message);
    return rVal;
}

// If wait_blocking is disabled, we block until we *receive* the message, but not after
// Otherwise, we block until the start time
void
RabbitMQ::waitForStartTime(bool skip_start_wait)
{
    auto message = consumeMessage();
    if (std::holds_alternative<StartTime>(message)) {
        if (skip_start_wait) {
            return;
        }

        StartTime start = std::get<StartTime>(message);
        log_i(
            wrapper_rabbitmq, "Received start time: {}, sleeping until then.",
            start.start_time_ns
        );

        std::chrono::high_resolution_clock::time_point wait_until =
            std::chrono::high_resolution_clock::time_point(
                std::chrono::nanoseconds(start.start_time_ns)
            );
        std::this_thread::sleep_until(wait_until);

        return;
    }
    else {
        log_e(wrapper_rabbitmq, "Received unexpected message type");
        exit(1);
    }
}

bool
RabbitMQ::initializeQueue(const std::string& queueName)
{
    amqp_queue_declare(
        conn, 1, amqp_cstring_bytes(queueName.c_str()), 0, 0, 0, 1, amqp_empty_table
    );

    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(wrapper_rabbitmq, "Failed to declare queue.");
        return false;
    }
    log_d(wrapper_rabbitmq, "Declared queue: {}", queueName);

    return true;
}

RabbitMQ::~RabbitMQ()
{
    amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);
}

} // namespace rabbitmq
} // namespace nutc
