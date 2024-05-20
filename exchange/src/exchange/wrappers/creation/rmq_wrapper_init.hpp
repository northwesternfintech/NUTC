#pragma once

#include "exchange/traders/trader_types/trader_interface.hpp"

namespace nutc {
namespace rabbitmq {
class WrapperInitializer {
public:
    using TraderPtr = std::shared_ptr<traders::GenericTrader>;
    static void
    send_start_time(const std::vector<TraderPtr>& traders, size_t wait_seconds);
};
} // namespace rabbitmq
} // namespace nutc
