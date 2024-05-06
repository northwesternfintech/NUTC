#pragma once

#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace rabbitmq {
class WrapperInitializer {
public:
    /**
     * @brief On startup, waits for all clients to send an initialization message
     *
     * This ensures that all clients are connected to the exchange and have successfully
     * started (vs a RMQ or firebase error)
     */
    static void wait_for_clients(traders::TraderContainer& manager);

    static void send_start_time(traders::TraderContainer& manager, size_t wait_seconds);
};
} // namespace rabbitmq
} // namespace nutc
