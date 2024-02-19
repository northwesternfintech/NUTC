#include "RabbitMQPublisher.hpp"

#include "exchange/logging.hpp"
#include "exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "exchange/traders/trader_types.hpp"

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
    const auto& active_clients = clients.get_traders();
    for (const auto& [id, trader] : active_clients) {
        for (const auto& match : matches) {
            if (trader->get_type() == manager::BOT)
                continue;
            if (!trader->is_active())
                continue;

            std::string buffer;
            glz::write<glz::opts{}>(match, buffer);
            publish_message(id, buffer);
        }
    }
}

void
RabbitMQPublisher::broadcast_ob_updates(
    const manager::ClientManager& clients,
    const std::vector<messages::ObUpdate>& updates, const std::string& ignore_uid
)
{
    const auto& traders = clients.get_traders();
    for (const auto& [id, trader] : traders) {
        if (trader->get_type() == manager::BOT)
            continue;
        if (!trader->is_active() || id == ignore_uid) {
            continue;
        }

        for (const auto& update : updates) {
            std::string buffer;
            glz::write<glz::opts{}>(update, buffer);
            publish_message(id, buffer);
        }
    }
}

void
RabbitMQPublisher::broadcast_account_update(
    const manager::ClientManager& clients, const messages::Match& match
)
{
    const std::string& buyer_id = match.buyer_id;
    const std::string& seller_id = match.seller_id;

    auto send_message = [&](const std::unique_ptr<manager::generic_trader_t>& trader,
                            messages::SIDE side) {
        if (trader->get_type() == manager::BOT)
            return;
        if (!trader->is_active())
            return;
        messages::AccountUpdate update = {
            match.ticker, side, match.price, match.quantity, trader->get_capital()
        };
        std::string buffer;
        glz::write<glz::opts{}>(update, buffer);
        publish_message(trader->get_id(), buffer);
    };

    send_message(clients.get_trader(buyer_id), messages::SIDE::BUY);
    send_message(clients.get_trader(seller_id), messages::SIDE::SELL);
}

} // namespace rabbitmq

} // namespace nutc
