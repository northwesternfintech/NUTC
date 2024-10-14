#include "runtime.hpp"

#include "wrapper/messaging/exchange_communicator.hpp"

#include <variant>

namespace nutc::wrapper {

void
Runtime::process_account_update(const account_update& update) const
{
    fire_on_account_update(
        update.trade.ticker, update.trade.side, update.trade.price,
        update.trade.quantity, update.available_capital
    );
}

void
Runtime::process_tick_update(const tick_update& update) const
{
    std::ranges::for_each(update.ob_updates, [&](const position& u) {
        fire_on_orderbook_update(u.ticker, u.side, u.quantity, u.price);
    });
    std::ranges::for_each(update.matches, [&](const common::position& position) {
        fire_on_trade_update(
            position.ticker, position.side, position.quantity, position.price
        );
    });
}

// Should not receive account update before we place an order
using start_tick_variant_t = std::variant<start_time, tick_update>;

void
Runtime::main_event_loop() const
{
    while (true) {
        auto update = ExchangeCommunicator::consume_market_update();
        if (std::holds_alternative<common::tick_update>(update)) [[likely]] {
            process_tick_update(std::get<common::tick_update>(update));
        }
        else if (std::holds_alternative<common::account_update>(update)) {
            process_account_update(std::get<common::account_update>(update));
        }
        else {
            throw std::runtime_error("Unknown update type received");
        }
    }
}
} // namespace nutc::wrapper
