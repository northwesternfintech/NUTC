#pragma once

#include "client_manager/client_manager.hpp"

namespace nutc {
namespace rabbitmq {
class RabbitMQClientManager {
public:
    /**
     * @brief On startup, waits for all clients to send an initialization message
     *
     * This ensures that all clients are connected to the exchange and have successfully
     * started (vs a RMQ or firebase error)
     */
    static void wait_for_clients(manager::ClientManager& manager, size_t num_clients);

    static void send_start_time(const manager::ClientManager& manager, int wait_seconds);
};
} // namespace rabbitmq
} // namespace nutc
