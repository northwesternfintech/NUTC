#include "runtime.hpp"

#include "wrapper/messaging/exchange_communicator.hpp"

namespace nutc::wrapper {

template <>
void
Runtime::process_message(start_time&)
{}

template <>
void
Runtime::process_message(tick_update&& tick_update)
{
    std::ranges::for_each(tick_update.ob_updates, [&](const position& u) {
        fire_on_orderbook_update(u.ticker, u.side, u.quantity, u.price);
    });
    std::ranges::for_each(tick_update.matches, [&](const match& m) {
        const auto& p = m.position;
        fire_on_trade_update(p.ticker, p.side, p.quantity, p.price);

        if (m.buyer_id == trader_id_) [[unlikely]] {
            fire_on_account_update(
                p.ticker, p.side, p.price, p.quantity, m.buyer_capital
            );
        }
        if (m.seller_id == trader_id_) [[unlikely]] {
            fire_on_account_update(
                p.ticker, p.side, p.price, p.quantity, m.seller_capital
            );
        }
    });
}

using start_tick_variant_t = std::variant<start_time, tick_update>;

void
Runtime::main_event_loop()
{
    while (true) {
        process_message(communicator_.consume_tick_update());
    }
}
} // namespace nutc::wrapper
