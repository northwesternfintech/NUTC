#include "consumer.hpp"

#include "exchange/concurrency/exchange_lock.hpp"
#include "exchange/logging.hpp"
#include "exchange/tick_scheduler/tick_scheduler.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <utility>
#include <variant>

namespace nutc {
namespace rabbitmq {

void
WrapperConsumer::on_tick(uint64_t)
{
    const auto& traders = traders::TraderContainer::get_instance().get_traders();
    auto& engine_manager = engine_manager::EngineManager::get_instance();

    for (const auto& pair : traders) {
        const auto& trader = pair.second;
        auto messages = trader->read_messages();
        for (auto message : messages) {
            auto handle_message = [&](auto&& arg) {
                using t = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<t, init_message>) {
                    handle_sandbox_init(trader, std::forward<t>(arg));
                }
                if constexpr (std::is_same_v<t, market_order>) {
                    match_new_order(engine_manager, trader, std::forward<t>(arg));
                }
            };

            std::visit(handle_message, message);
        }
    }
}

void
WrapperConsumer::handle_sandbox_init(
    const std::shared_ptr<traders::GenericTrader>& trader, init_message&& message
)
{
    auto& trader_container = traders::TraderContainer::get_instance();
    if (!message.ready) {
        log_i(
            main, "Sandbox wrapper with id {} reported bad initialization",
            trader->get_id()
        );
        // TODO
        trader_container.get_instance().remove_trader(trader->get_id());
        return;
    }
    trader->send_messages({glz::write_json(messages::start_time{0})});
    log_i(main, "Sandbox wrapper with id {} fully initialized", trader->get_id());
}

void
WrapperConsumer::match_new_order(
    engine_manager::EngineManager& engine_manager,
    const std::shared_ptr<traders::GenericTrader>& trader, market_order&& order
)
{
    if (order.price < 0 || order.quantity <= 0) {
        assert(false);
        trader->process_order_expiration(
            order.ticker, order.side, order.price, order.quantity
        );
        return;
    }
    else if (!engine_manager.has_engine(order.ticker)) {
        assert(false);
        return;
    }

    auto current_tick = ticks::TickJobScheduler::get().get_current_tick();
    auto stored_order =
        matching::stored_order{trader,         order.side,  order.ticker,
                               order.quantity, order.price, current_tick};

    engine_manager.match_order(stored_order);
}

} // namespace rabbitmq
} // namespace nutc
