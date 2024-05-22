#pragma once

#include "exchange/tick_scheduler/tick_observer.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

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

    void on_tick(uint64_t new_tick) override;

    static void match_new_order(
        engine_manager::EngineManager& engine_manager,
        const std::shared_ptr<traders::GenericTrader>& trader, market_order&& order
    );
};

} // namespace rabbitmq
} // namespace nutc
