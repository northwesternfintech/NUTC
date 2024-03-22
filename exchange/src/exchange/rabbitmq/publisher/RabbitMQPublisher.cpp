#include "RabbitMQPublisher.hpp"

#include "exchange/logging.hpp"
#include "exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"

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

// TODO: make this clear it also publishes account updates
void
RabbitMQPublisher::broadcast_matches(
    const manager::TraderManager& clients, const std::vector<messages::Match>& matches
)
{
    const auto& active_clients = clients.get_traders();
    for (const auto& [id, trader] : active_clients) {
        for (const auto& match : matches) {
            if (trader->get_type() == manager::TraderType::BOT)
                continue;
            if (!trader->is_active())
                continue;

            std::string buffer;
            glz::write<glz::opts{}>(match, buffer);
            publish_message(id, buffer);

            messages::AccountUpdate update{
                match.ticker, match.side, match.price, match.quantity,
                clients.get_trader(id)->get_capital()
            };
            glz::write<glz::opts{}>(update, buffer);
            publish_message(id, buffer);
        }
    }
}

void
RabbitMQPublisher::broadcast_ob_updates(
    const manager::TraderManager& clients,
    const std::vector<messages::ObUpdate>& updates
)
{
    const auto& traders = clients.get_traders();
    for (const auto& [id, trader] : traders) {
        if (trader->get_type() == manager::TraderType::BOT)
            continue;
        if (!trader->is_active()) {
            continue;
        }

        for (const auto& update : updates) {
            std::string buffer;
            glz::write<glz::opts{}>(update, buffer);
            publish_message(id, buffer);
        }
    }
}

} // namespace rabbitmq

} // namespace nutc
