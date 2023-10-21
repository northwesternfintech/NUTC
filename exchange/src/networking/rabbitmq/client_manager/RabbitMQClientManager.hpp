#pragma once

#include "client_manager/client_manager.hpp"

namespace nutc {
namespace rabbitmq {
//TODO: refactor to client_manager?

class RabbitMQClientManager {
public:
    /**
     * @brief On startup, waits for all clients to send an initialization message
     *
     * This ensures that all clients are connected to the exchange and have successfully
     * started (vs a RMQ or firebase error)
     */
    static void waitForClients(manager::ClientManager& manager, int num_clients);

    static void sendStartTime(const manager::ClientManager& manager, int wait_seconds);
};
} // namespace rabbitmq
} // namespace nutc
