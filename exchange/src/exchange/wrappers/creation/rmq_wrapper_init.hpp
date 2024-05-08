#pragma once

#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace rabbitmq {
class WrapperInitializer {
public:
    static void send_start_time(traders::TraderContainer& manager, size_t wait_seconds);
};
} // namespace rabbitmq
} // namespace nutc
