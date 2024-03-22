#include "RabbitMQPublisher.hpp"

#include "exchange/logging.hpp"
#include "exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"

namespace nutc {
namespace rabbitmq {

bool
RabbitMQPublisher::publish_message(
    const std::string& target_name, const std::string& message, bool is_exchange
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

    if (is_exchange) {
        amqp_basic_publish(
            conn, 1, amqp_cstring_bytes(target_name.c_str()), amqp_cstring_bytes(""), 0,
            0, nullptr, amqp_cstring_bytes(message.c_str())
        );
    }
    else {
        amqp_basic_publish(
            conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes(target_name.c_str()), 0,
            0, nullptr, amqp_cstring_bytes(message.c_str())
        );
    }

    return check_reply(amqp_get_rpc_reply(conn), "Failed to publish message.");
}

// TODO: make this clear it also publishes account updates
void
RabbitMQPublisher::broadcast_matches(
    const manager::TraderManager& clients, const std::vector<messages::Match>& matches
)
{
    for (const auto& match : matches) {
        std::string buffer;
        glz::write<glz::opts{}>(match, buffer);
        publish_message("fanout_to_wrappers", buffer, /*is_exchange=*/true);

        messages::AccountUpdate update1{
            match.ticker, match.side, match.price, match.quantity,
            clients.get_trader(match.buyer_id)->get_capital()
        };
        messages::AccountUpdate update2{
            match.ticker, match.side, match.price, match.quantity,
            clients.get_trader(match.seller_id)->get_capital()
        };
        glz::write<glz::opts{}>(update1, buffer);
        publish_message(match.buyer_id, buffer);

        glz::write<glz::opts{}>(update2, buffer);
        publish_message(match.seller_id, buffer);
    }
}

void
RabbitMQPublisher::broadcast_ob_updates(const std::vector<messages::ObUpdate>& updates)
{
    for (const auto& update : updates) {
        std::string buffer;
        glz::write<glz::opts{}>(update, buffer);
        publish_message("fanout_to_wrappers", buffer, /*is_exchange=*/true);
    }
}

} // namespace rabbitmq

} // namespace nutc
