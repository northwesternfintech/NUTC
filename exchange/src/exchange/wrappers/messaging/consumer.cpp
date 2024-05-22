#include "consumer.hpp"

#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace rabbitmq {

void
WrapperConsumer::on_tick(uint64_t current_tick)
{
    const auto& traders = traders::TraderContainer::get_instance().get_traders();
    std::vector<market_order> orders;

    for (const auto& trader : traders) {
        auto messages = trader->read_orders();
        for (auto order : messages) {
            matching::stored_order stored_order{trader,       order.side,
                                                order.ticker, order.quantity,
                                                order.price,  current_tick};
            match_order(stored_order);
        }
    }
}

void
WrapperConsumer::match_order(const matching::stored_order& order)
{
    if (order.price < 0 || order.quantity <= 0) {
        return;
    }
    else if (!manager_->has_engine(order.ticker)) {
        return;
    }

    manager_->match_order(order);
}

} // namespace rabbitmq
} // namespace nutc
