#include "RabbitMQPublisher.hpp"

#include "logging.hpp"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"

namespace nutc {
namespace rabbitmq {

bool
RabbitMQPublisher::publishMessage(
    const std::string& queueName, const std::string& message
)
{
    auto checkReply = [&](amqp_rpc_reply_t reply, const char* errorMsg) -> bool {
        if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
            log_e(rabbitmq, "{}", errorMsg);
            return false;
        }
        return true;
    };

    const auto& conn = RabbitMQConnectionManager::getInstance().get_connection_state();

    if (!checkReply(amqp_get_rpc_reply(conn), "Failed to declare queue.")) {
        return false;
    }

    amqp_basic_publish(
        conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes(queueName.c_str()), 0, 0,
        nullptr, amqp_cstring_bytes(message.c_str())
    );

    return checkReply(amqp_get_rpc_reply(conn), "Failed to publish message.");
}

void
RabbitMQPublisher::broadcastMatches(
    const manager::ClientManager& clients, const std::vector<messages::Match>& matches
)
{
    auto broadcastToClient = [&](const std::pair<std::string, manager::client>& pair) {
        for (const auto& match : matches) {
            const auto& [id, client] = pair;

            if (!client.active)
                continue;

            std::string buffer;
            glz::write<glz::opts{}>(match, buffer);
            publishMessage(id, buffer);
        }
    };

    const auto& activeClients = clients.get_clients();
    std::for_each(activeClients.begin(), activeClients.end(), broadcastToClient);
}

void
RabbitMQPublisher::broadcastObUpdates(
    const manager::ClientManager& clients,
    const std::vector<messages::ObUpdate>& updates, const std::string& ignore_id
)
{
    auto broadcastToClient = [&](const std::pair<std::string, manager::client>& pair) {
        const auto& [id, client] = pair;

        if (!client.active || id == ignore_id) {
            return;
        }

        for (const auto& update : updates) {
            std::string buffer;
            glz::write<glz::opts{}>(update, buffer);
            publishMessage(id, buffer);
        }
    };

    const auto& activeClients = clients.get_clients();
    std::for_each(activeClients.begin(), activeClients.end(), broadcastToClient);
}

void
RabbitMQPublisher::broadcastAccountUpdate(
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
    publishMessage(buyer_id, buyer_buffer);
    publishMessage(seller_id, seller_buffer);
}

} // namespace rabbitmq

} // namespace nutc
