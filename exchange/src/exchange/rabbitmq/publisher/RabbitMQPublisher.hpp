#pragma once

#include "exchange/traders/trader_manager.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <string>

namespace nutc {
namespace rabbitmq {

class RabbitMQPublisher {
public:
    static bool publish_message(
        const std::string& target_name, const std::string& message, bool is_exchange
    );

    static bool
    publish_message(const std::string& target_name, const std::string& message)
    {
        return publish_message(target_name, message, /*is_exchange=*/false);
    }

    static void broadcast_matches(
        const manager::TraderManager& clients,
        const std::vector<messages::Match>& matches
    );

    // ignore id because we shouldn't send ob update to user who placed order
    static void broadcast_ob_updates(const std::vector<messages::ObUpdate>& updates);
};

} // namespace rabbitmq
} // namespace nutc
