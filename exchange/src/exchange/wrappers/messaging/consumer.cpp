#include "consumer.hpp"

#include "exchange/tick_scheduler/tick_scheduler.hpp"
#include "exchange/traders/trader_container.hpp"

#include <utility>

namespace nutc {
namespace rabbitmq {

void
WrapperConsumer::on_tick(uint64_t)
{
    const auto& traders = traders::TraderContainer::get_instance().get_traders();
    std::vector<market_order> orders;

    for (const auto& trader : traders) {
        auto messages = trader->read_orders();
        for (auto order : messages) {
            match_new_order(*manager_, trader, std::move(order));
        }
    }
}

void
WrapperConsumer::match_new_order(
    engine_manager::EngineManager& engine_manager,
    const std::shared_ptr<traders::GenericTrader>& trader, market_order&& order
)
{
    if (order.price < 0 || order.quantity <= 0) {
        trader->process_order_remove(order);
        return;
    }
    else if (!engine_manager.has_engine(order.ticker)) {
        return;
    }

    auto current_tick = ticks::TickJobScheduler::get().get_current_tick();
    matching::stored_order stored_order{trader,         order.side,  order.ticker,
                                        order.quantity, order.price, current_tick};

    engine_manager.match_order(stored_order);
}

} // namespace rabbitmq
} // namespace nutc
