#include "rabbitmq.hpp"

#include "logging.hpp"

namespace nutc {
namespace rabbitmq {
RabbitMQ::RabbitMQ(manager::ClientManager& manager) : clients(manager) {}

bool
RabbitMQ::logAndReturnError(const char* errorMessage)
{
    log_e(rabbitmq, "{}", errorMessage);
    return false;
}

bool
RabbitMQ::connectToRabbitMQ(
    const std::string& hostname, int port, const std::string& username,
    const std::string& password
)
{
    conn = amqp_new_connection();
    amqp_socket_t* socket = amqp_tcp_socket_new(conn);

    if (!socket)
        return logAndReturnError("Cannot create TCP socket");
    if (amqp_socket_open(socket, hostname.c_str(), port))
        return logAndReturnError("Cannot open socket");

    amqp_rpc_reply_t reply = amqp_login(
        conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, username.c_str(),
        password.c_str()
    );
    if (reply.reply_type != AMQP_RESPONSE_NORMAL)
        return logAndReturnError("Login failed");

    return true;
}

bool
RabbitMQ::initializeQueue(const std::string& queueName)
{
    amqp_queue_declare(
        conn, 1, amqp_cstring_bytes(queueName.c_str()), 0, 0, 0, 1, amqp_empty_table
    );
    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);

    if (res.reply_type != AMQP_RESPONSE_NORMAL)
        return logAndReturnError("Failed to declare queue.");
    log_i(rabbitmq, "Declared queue: {}", queueName);

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
        log_e(rabbitmq, "Failed to consume message.");
        return false;
    }

    return true;
}

void
RabbitMQ::handleIncomingMessages()
{
    // need condition for closing
    while (true) {
        std::variant<InitMessage, MarketOrder, RMQError> incoming_message =
            consumeMessage();
        bool is_init = std::holds_alternative<InitMessage>(incoming_message);
        bool is_mo = std::holds_alternative<MarketOrder>(incoming_message);
        bool is_error = std::holds_alternative<RMQError>(incoming_message);
        if (is_init) [[unlikely]] {
            log_e(rabbitmq, "Not expecting initialization message");
            exit(1);
        }
        if (is_error) [[unlikely]] {
            RMQError err = std::get<RMQError>(incoming_message);
            log_e(rabbitmq, "Received RMQError: {}", err.message);
        }
        if (is_mo) [[likely]] {
            MarketOrder mo = std::get<MarketOrder>(incoming_message);
            handleIncomingMarketOrder(mo);
        }
    }
}

void
RabbitMQ::addTicker(const std::string& ticker)
{
    engine_manager.addEngine(ticker);
}

void
RabbitMQ::handleIncomingMarketOrder(const MarketOrder& order)
{
    std::string buffer;
    glz::write<glz::opts{}>(order, buffer);
    std::string replace1 = R"("side":0)";
    std::string replace2 = R"("side":1)";
    size_t pos1 = buffer.find(replace1);
    size_t pos2 = buffer.find(replace2);
    if (pos1 != std::string::npos) {
        buffer.replace(pos1, replace1.length(), R"("side":"buy")");
    }
    if (pos2 != std::string::npos) {
        buffer.replace(pos2, replace2.length(), R"("side":"ask")");
    }

    log_i(rabbitmq, "Received market order: {}", buffer);
    std::optional<matching::Engine> engine = engine_manager.getEngine(order.ticker);
    if (!engine.has_value()) {
        log_w(
            matching, "Received order for unknown ticker {}. Discarding order",
            order.ticker
        );
        return;
    }
    auto [matches, ob_updates] = engine.value().match_order(order);
    for (const auto& match : matches) {
        std::string buyer_uid = match.buyer_uid;
        std::string seller_uid = match.seller_uid;
        float capital_exchanged = match.price * match.quantity;
        clients.modifyCapital(buyer_uid, -capital_exchanged);
        clients.modifyCapital(seller_uid, capital_exchanged);
        broadcastAccountUpdate(match);
        log_i(
            matching, "Matched order with price {} and quantity {}", match.price,
            match.quantity
        );
    }
    for (const auto& update : ob_updates) {
        log_i(
            rabbitmq, "New ObUpdate with ticker {} price {} quantity {} side {}",
            update.security, update.price, update.quantity,
            update.side == messages::SIDE::BUY ? "BUY" : "ASK"
        );
    }
    if (matches.size() > 0) {
        broadcastMatches(matches);
    }
    if (ob_updates.size() > 0) {
        broadcastObUpdates(ob_updates);
    }
}

void
RabbitMQ::broadcastAccountUpdate(const Match& match)
{
    std::string buyer_uid = match.buyer_uid;
    std::string seller_uid = match.seller_uid;
    AccountUpdate buyer_update = {
        clients.getCapital(match.buyer_uid), match.ticker, messages::SIDE::BUY,
        match.price, match.quantity
    };
    AccountUpdate seller_update = {
        clients.getCapital(match.seller_uid), match.ticker, messages::SIDE::SELL,
        match.price, match.quantity
    };

    std::string buyer_buffer;
    std::string seller_buffer;
    glz::write<glz::opts{}>(buyer_update, buyer_buffer);
    glz::write<glz::opts{}>(seller_update, seller_buffer);
    publishMessage(buyer_uid, buyer_buffer);
    publishMessage(seller_uid, seller_buffer);
}

void
RabbitMQ::broadcastObUpdates(const std::vector<ObUpdate>& updates)
{
    for (auto& [uid, active, capital_remaining] : clients.getClients(true)) {
        for (auto& update : updates) {
            // todo: eliminate for loop
            std::string buffer;
            glz::write<glz::opts{}>(update, buffer);
            publishMessage(uid, buffer);
        }
    }
}

void
RabbitMQ::broadcastMatches(const std::vector<Match>& matches)
{
    for (auto& [uid, active, capital_remaining] : clients.getClients(true)) {
        for (auto& match : matches) {
            // todo: eliminate for loop
            std::string buffer;
            glz::write<glz::opts{}>(match, buffer);
            publishMessage(uid, buffer);
        };
    };
}

bool
RabbitMQ::publishMessage(const std::string& queueName, const std::string& message)
{
    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to declare queue.");
        return false;
    }

    amqp_basic_publish(
        conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes(queueName.c_str()), 0, 0,
        NULL, amqp_cstring_bytes(message.c_str())
    );

    res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        log_e(rabbitmq, "Failed to publish message.");
        return false;
    }

    return true;
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

std::variant<InitMessage, MarketOrder, RMQError>
RabbitMQ::consumeMessage()
{
    std::string buf = consumeMessageAsString();
    if (buf == "") {
        return RMQError{"Failed to consume message."};
    }

    std::variant<InitMessage, MarketOrder, RMQError> data{};
    auto err = glz::read_json(data, buf);
    if (err) {
        std::string error = glz::format_error(err, buf);
        return RMQError{error};
    }
    return data;
}

// todo: find way to prevent clients from starting until all are ready
void
RabbitMQ::waitForClients(int num_clients)
{
    for (int i = 0; i < num_clients; i++) {
        std::variant<InitMessage, MarketOrder, RMQError> data = consumeMessage();
        if (std::holds_alternative<RMQError>(data)) {
            std::string error = std::get<RMQError>(data).message;
            log_e(rabbitmq, "Failed to consume message with error {}.", error);
            return;
        }
        if (std::holds_alternative<MarketOrder>(data)) {
            log_i(
                rabbitmq,
                "Received market order before initialization complete. Ignoring..."
            );
            continue;
        }
        InitMessage init = std::get<InitMessage>(data);
        log_i(
            rabbitmq, "Received init message from client {} with status {}",
            init.client_uid, init.ready ? "ready" : "not ready"
        );
        if (init.ready) {
            clients.setClientActive(init.client_uid);
        }
    }
    log_i(rabbitmq, "All clients ready. Starting exchange");
}

bool
RabbitMQ::initializeConnection()
{
    if (!connectToRabbitMQ("localhost", 5672, "NUFT", "ADMIN"))
        return false;

    amqp_channel_open(conn, 1);
    amqp_rpc_reply_t res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL)
        return logAndReturnError("Failed to open channel.");

    if (!initializeQueue("market_order"))
        return false;
    if (!initializeConsume("market_order"))
        return false;

    return true;
}

void
RabbitMQ::closeConnection()
{
    for (auto& [uid, active, capital_remaining] : clients.getClients(true)) {
        log_i(rabbitmq, "Shutting down client {}", uid);
        ShutdownMessage shutdown{uid};
        std::string mess = glz::write_json(shutdown);
        publishMessage(uid, mess);
    }
    amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);
}

} // namespace rabbitmq
} // namespace nutc
