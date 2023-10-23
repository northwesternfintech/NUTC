#include "RabbitMQPublisher.hpp"

#include "logging.hpp"
#include "networking/rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"

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
    auto broadcastToClient = [&](const auto& client) {
        for (const auto& match : matches) {
            std::string buffer;
            glz::write<glz::opts{}>(match, buffer);
            publishMessage(client.uid, buffer);
        }
    };

    const auto activeClients = clients.get_clients(true);
    std::for_each(activeClients.begin(), activeClients.end(), broadcastToClient);
}

void
RabbitMQPublisher::broadcastObUpdates(
    const manager::ClientManager& clients,
    const std::vector<messages::ObUpdate>& updates, const std::string& ignore_uid
)
{
    auto broadcastToClient = [&](const auto& client) {
        if (client.uid == ignore_uid) {
            return;
        }
        for (const auto& update : updates) {
            // if (update.quantity <= 1e-6f) {
            // continue;
            // }
            std::string buffer;
            glz::write<glz::opts{}>(update, buffer);
            publishMessage(client.uid, buffer);
        }
    };

    const auto activeClients = clients.get_clients(true);
    std::for_each(activeClients.begin(), activeClients.end(), broadcastToClient);
}

void
RabbitMQPublisher::broadcastAccountUpdate(
    const manager::ClientManager& clients, const messages::Match& match
)
{
    std::string buyer_uid = match.buyer_uid;
    std::string seller_uid = match.seller_uid;
    messages::AccountUpdate buyer_update = {
        clients.get_capital(match.buyer_uid), match.ticker, messages::SIDE::BUY,
        match.price, match.quantity
    };
    messages::AccountUpdate seller_update = {
        clients.get_capital(match.seller_uid), match.ticker, messages::SIDE::SELL,
        match.price, match.quantity
    };

    std::string buyer_buffer;
    std::string seller_buffer;
    glz::write<glz::opts{}>(buyer_update, buyer_buffer);
    glz::write<glz::opts{}>(seller_update, seller_buffer);
    publishMessage(buyer_uid, buyer_buffer);
    publishMessage(seller_uid, seller_buffer);
}

} // namespace rabbitmq

} // namespace nutc
