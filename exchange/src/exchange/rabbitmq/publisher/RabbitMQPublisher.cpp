#include "RabbitMQPublisher.hpp"

#include "exchange/logging.hpp"
#include "exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"

namespace nutc {
namespace rabbitmq {

bool
RabbitMQPublisher::publish_message(
    const std::string& queue_name, const std::string& message
)
{
    auto check_reply = [&](amqp_rpc_reply_t reply, const char* error_msg) -> bool {
        if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
            log_e(rabbitmq, "{}", error_msg);
            return false;
        }
        return true;
    };

    const auto& conn = RabbitMQConnectionManager::get_instance().get_connection_state();

    if (!check_reply(amqp_get_rpc_reply(conn), "Failed to declare queue.")) {
        return false;
    }

    amqp_basic_publish(
        conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes(queue_name.c_str()), 0, 0,
        nullptr, amqp_cstring_bytes(message.c_str())
    );

    return check_reply(amqp_get_rpc_reply(conn), "Failed to publish message.");
}

void
RabbitMQPublisher::broadcast_matches(
    const manager::ClientManager& clients, const std::vector<messages::Match>& matches
)
{
    auto broadcast_to_client = [&](const std::pair<std::string, manager::client_t>& pair
                               ) {
        for (const auto& match : matches) {
            const auto& [id, client] = pair;

            if (!client.active)
                continue;

            std::string buffer;
            glz::write<glz::opts{}>(match, buffer);
            publish_message(id, buffer);
        }
    };

    const auto& active_clients = clients.get_clients();
    std::for_each(active_clients.begin(), active_clients.end(), broadcast_to_client);
}

void
RabbitMQPublisher::broadcast_ob_updates(
    const manager::ClientManager& clients,
    const std::vector<messages::ObUpdate>& updates, const std::string& ignore_uid
)
{
    auto broadcast_to_client = [&](const std::pair<std::string, manager::client_t>& pair
                               ) {
        const auto& [id, client] = pair;

        if (!client.active || id == ignore_uid) {
            return;
        }

        for (const auto& update : updates) {
            std::string buffer;
            glz::write<glz::opts{}>(update, buffer);
            publish_message(id, buffer);
        }
    };

    const auto& active_clients = clients.get_clients();
    std::for_each(active_clients.begin(), active_clients.end(), broadcast_to_client);
}

void
RabbitMQPublisher::broadcast_account_update(
    const manager::ClientManager& clients, const messages::Match& match
)
{
    const std::string& buyer_id = match.buyer_id;
    const std::string& seller_id = match.seller_id;

    messages::AccountUpdate buyer_update = {
        match.ticker,   messages::SIDE::BUY,           match.price,
        match.quantity, clients.get_capital(buyer_id),
    };
    messages::AccountUpdate seller_update = {
        match.ticker,   messages::SIDE::SELL,           match.price,
        match.quantity, clients.get_capital(seller_id),
    };

    std::string buyer_buffer;
    std::string seller_buffer;
    glz::write<glz::opts{}>(buyer_update, buyer_buffer);
    glz::write<glz::opts{}>(seller_update, seller_buffer);
    publish_message(buyer_id, buyer_buffer);
    publish_message(seller_id, seller_buffer);
}

} // namespace rabbitmq

} // namespace nutc
