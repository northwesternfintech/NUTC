#pragma once

#include "exchange/client_manager/client_manager.hpp"
#include "exchange/utils/messages.hpp"

#include <string>

namespace nutc {
namespace rabbitmq {

class RabbitMQPublisher {
public:
    static bool
    publish_message(const std::string& queue_name, const std::string& message);
    static void broadcast_matches(
        const manager::ClientManager& clients,
        const std::vector<messages::Match>& matches
    );

    // ignore id because we shouldn't send ob update to user who placed order
    static void broadcast_ob_updates(
        const manager::ClientManager& clients,
        const std::vector<messages::ObUpdate>& updates, const std::string& ignore_uid
    );
    static void broadcast_account_update(
        const manager::ClientManager& clients, const messages::Match& match
    );
};

} // namespace rabbitmq
} // namespace nutc
