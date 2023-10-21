#pragma once

#include "client_manager/client_manager.hpp"
#include "utils/messages.hpp"

#include <string>

namespace nutc {
namespace rabbitmq {

class RabbitMQPublisher {
public:
    // TODO: should take in variant of messages
    static bool publishMessage(const std::string& queueName, const std::string& message);
    static void broadcastMatches(
        const manager::ClientManager& clients,
        const std::vector<messages::Match>& matches
    );

    // ignore uid because we shouldn't send ob update to user who placed order
    static void broadcastObUpdates(
        const manager::ClientManager& clients,
        const std::vector<messages::ObUpdate>& updates, const std::string& ignore_uid
    );
    static void broadcastAccountUpdate(
        const manager::ClientManager& clients, const messages::Match& match
    );
};

} // namespace rabbitmq
} // namespace nutc
