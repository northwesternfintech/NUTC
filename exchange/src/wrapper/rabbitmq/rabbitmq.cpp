#include "rabbitmq.hpp"

#include "wrapper/logging.hpp"
#include "wrapper/pywrapper/pywrapper.hpp"

#include <chrono>

#include <iostream>

namespace nutc {
namespace rabbitmq {

bool
RabbitMQHandler::bind_queue_to_exchange(
    const std::string& queue_name, const std::string& exchange_name
)
{
    amqp_queue_bind(
        conn, 1, amqp_cstring_bytes(queue_name.c_str()),
        amqp_cstring_bytes(exchange_name.c_str()), amqp_empty_bytes, amqp_empty_table
    );
    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);

    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(wrapper_rabbitmq, "Failed to bind queue to exchange.");
        return false;
    }
    log_i(wrapper_rabbitmq, "Bound queue {} to exchange {}", queue_name, exchange_name);

    return true;
}

bool
RabbitMQHandler::connect_to_rmq(
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
RabbitMQHandler::main_event_loop(const std::string& uid)
{
    while (true) {
        std::variant<start_time, orderbook_update, match> data = consume_message();
        std::visit(
            [&](auto&& arg) { process_message(std::forward<decltype(arg)>(arg), uid); },
            std::move(data)
        );
    }
}

template <typename T>
void
RabbitMQHandler::process_message(T&& message, const std::string& uid)
{
    using MessageType = std::decay_t<T>;
    if constexpr (std::is_same_v<MessageType, orderbook_update>) {
        handle_orderbook_update(message);
    }
    else if constexpr (std::is_same_v<MessageType, match>) {
        handle_match(message, uid);
    }
}

void
RabbitMQHandler::handle_orderbook_update(const orderbook_update& update)
{
    std::string side = update.side == util::Side::buy ? "BUY" : "SELL";
    nutc::pywrapper::get_ob_update_function()(
        update.ticker, side, update.price, update.quantity
    );
}

void
RabbitMQHandler::handle_match(const match& match, const std::string& uid)
{
    std::string side = match.side == util::Side::buy ? "BUY" : "SELL";

    nutc::pywrapper::get_trade_update_function()(
        match.ticker, side, match.price, match.quantity
    );

    if (match.buyer_id == uid) {
        nutc::pywrapper::get_account_update_function()(
            match.ticker, side, match.price, match.quantity, match.buyer_capital
        );
    }
    else if (match.seller_id == uid) {
        nutc::pywrapper::get_account_update_function()(
            match.ticker, side, match.price, match.quantity, match.seller_capital
        );
    }
}

bool
RabbitMQHandler::publish_market_order(
    const std::string& client_id, const std::string& side, const std::string& ticker,
    double quantity, double price
)
{
    if (limiter.should_rate_limit()) {
        return false;
    }
    market_order order{
        client_id, side == "BUY" ? util::Side::buy : util::Side::sell, ticker, quantity,
        price
    };
    std::string message = glz::write_json(order);

    return publish_message("market_order", message);
}

bool
RabbitMQHandler::publish_message(
    const std::string& queueName, const std::string& message
)
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

std::variant<start_time, orderbook_update, match>
RabbitMQHandler::consume_message()
{
    std::string buf{};
    std::getline(std::cin, buf);
    while (buf.empty()) {
        std::getline(std::cin, buf);
        throw std::runtime_error("Wrapper failed to consume message.");
    }
    std::cout << buf << std::endl;

    std::variant<start_time, orderbook_update, match> data{};
    auto err = glz::read_json(data, buf);
    if (err) {
        std::string error = glz::format_error(err, buf);
        throw std::runtime_error(fmt::format("Failed to parse message: {}", error));
    }
    return data;
}

// Blocking
std::string
RabbitMQHandler::consume_message_as_string()
{
    amqp_envelope_t envelope;
    amqp_maybe_release_buffers(conn);
    amqp_rpc_reply_t res = amqp_consume_message(conn, &envelope, NULL, 0);

    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(wrapper_rabbitmq, "Wrapper failed to consume message.");
        return "";
    }

    std::string message(
        static_cast<char*>(envelope.message.body.bytes), envelope.message.body.len
    );
    amqp_destroy_envelope(&envelope);
    return message;
}

bool
RabbitMQHandler::initialize_connection(const std::string& queueName)
{
    if (!connect_to_rmq("localhost", 5672, "NUFT", "ADMIN")) {
        log_c(wrapper_rabbitmq, "Failed to connect to RabbitMQ");
        return false;
    }
    amqp_channel_open(conn, 1);
    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(wrapper_rabbitmq, "Failed to open channel.");
        return false;
    }

    if (!initialize_queue(queueName)) {
        return false;
    }

    if (!bind_queue_to_exchange(queueName, "fanout_to_wrappers")) {
        return false;
    }

    if (!initialize_consume(queueName)) {
        return false;
    }

    return true;
}

bool
RabbitMQHandler::initialize_consume(const std::string& queueName)
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

RabbitMQHandler::RabbitMQHandler(const std::string& user_id)
{
    if (!initialize_connection(user_id)) {
        log_c(wrapper_rabbitmq, "Failed to initialize connection to RabbitMQ");
        // attempt to say we didn't init correctly
        bool published_init = publish_init_message(user_id, /*ready=*/false);
        if (!published_init) {
            log_e(wrapper_rabbitmq, "Failed to publish init message");
        }

        exit(1);
    }
}

std::function<bool(const std::string&, const std::string&, double, double)>
RabbitMQHandler::market_order_func(const std::string& user_id)
{
    return [&](const std::string& side, const auto& ticker, const auto& quantity,
               const auto& price) {
        return RabbitMQHandler::publish_market_order(
            user_id, side, ticker, quantity, price
        );
    };
}

bool
RabbitMQHandler::publish_init_message(const std::string& user_id, bool ready)
{
    std::string message = glz::write_json(init_message{user_id, ready});
    return publish_message("market_order", message);
}

// If wait_blocking is disabled, we block until we *receive* the message, but not
// after Otherwise, we block until the start time
void
RabbitMQHandler::wait_for_start_time(bool skip_start_wait)
{
    auto message = consume_message();
    if (!std::holds_alternative<start_time>(message))
        throw std::runtime_error("Received unexpected message type");

    if (skip_start_wait)
        return;

    start_time start = std::get<start_time>(message);
    log_i(
        wrapper_rabbitmq, "Received start time: {}, sleeping until then.",
        start.start_time_ns
    );

    std::chrono::high_resolution_clock::time_point wait_until =
        std::chrono::high_resolution_clock::time_point(
            std::chrono::nanoseconds(start.start_time_ns)
        );
    std::this_thread::sleep_until(wait_until);

    log_i(wrapper_rabbitmq, "Done sleeping, start time: {}", start.start_time_ns);
}

bool
RabbitMQHandler::initialize_queue(const std::string& queue_name)
{
    amqp_queue_declare(
        conn, 1, amqp_cstring_bytes(queue_name.c_str()), 0, 0, 0, 1, amqp_empty_table
    );

    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(wrapper_rabbitmq, "Failed to declare queue.");
        return false;
    }
    log_d(wrapper_rabbitmq, "Declared queue: {}", queue_name);

    return true;
}

RabbitMQHandler::~RabbitMQHandler()
{
    amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);
}

} // namespace rabbitmq
} // namespace nutc
