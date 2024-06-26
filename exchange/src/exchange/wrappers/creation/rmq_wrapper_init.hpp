#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

namespace nutc {
namespace rabbitmq {
class WrapperInitializer {
public:
    using TraderPtr = std::shared_ptr<traders::GenericTrader>;
    static void
    send_start_time(const std::pmr::vector<TraderPtr>& traders, size_t wait_seconds);
};
} // namespace rabbitmq
} // namespace nutc
