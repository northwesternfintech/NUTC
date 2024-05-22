#pragma once

#include "exchange/tick_scheduler/tick_observer.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"

namespace nutc {
namespace rabbitmq {
using market_order = messages::market_order;
using init_message = messages::init_message;

class WrapperConsumer : public ticks::TickObserver {
    std::shared_ptr<engine_manager::EngineManager> manager_;

public:
    WrapperConsumer(std::shared_ptr<engine_manager::EngineManager> manager) :
        manager_(manager)
    {}

    void on_tick(uint64_t current_tick) override;

private:
    void match_order(const matching::stored_order& order);
};

} // namespace rabbitmq
} // namespace nutc
