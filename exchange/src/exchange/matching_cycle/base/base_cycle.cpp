#include "base_cycle.hpp"

#include "common/messages_exchange_to_wrapper.hpp"
#include "common/messages_wrapper_to_exchange.hpp"
#include "exchange/matching/engine.hpp"

#include <variant>

namespace nutc::exchange {

void
BaseMatchingCycle::before_cycle_(uint64_t)
{
    for (auto [symbol, ticker_data] : tickers_) {
        ticker_data.generate_bot_orders();
    }
}

// TODO: clean up with new cancel_order logic
auto
BaseMatchingCycle::collect_orders(uint64_t) -> std::vector<OrderVariant>
{
    std::vector<OrderVariant> orders;

    auto collect_orders = [&orders](GenericTrader& trader) {
        auto message_queue = trader.read_orders();

        auto get_tagged_order = [&trader]<typename OrderT>(const OrderT& order)
            -> std::variant<
                common::cancel_order, tagged_limit_order, tagged_market_order> {
            if constexpr (std::is_same_v<OrderT, common::init_message>) {
                throw std::runtime_error("Unexpected initialization message");
            }
            else if constexpr (std::is_same_v<OrderT, common::cancel_order>) {
                return order;
            }
            else {
                return tagged_order<OrderT>{trader, order};
            }
        };

        std::transform(
            message_queue.begin(), message_queue.end(), std::back_inserter(orders),
            [&](auto& order) { return std::visit(get_tagged_order, order); }
        );
    };

    std::for_each(traders_.begin(), traders_.end(), collect_orders);

    return orders;
}

std::vector<common::match>
BaseMatchingCycle::match_orders_(std::vector<OrderVariant> orders)
{
    std::vector<common::match> matches;

    for (OrderVariant& order_variant : orders) {
        auto match_incoming_order = [&]<typename OrderT>(OrderT& order) {
            auto& ticker_data = tickers_[order.ticker];
            auto& orderbook = ticker_data.get_orderbook();
            if constexpr (std::is_same_v<OrderT, common::cancel_order>) {
                orderbook.remove_order(order.order_id);
            }
            else {
                if (order.quantity <= 0.0)
                    return;
                auto tmp = match_order(order, orderbook, order_fee_);
                std::copy(tmp.begin(), tmp.end(), std::back_inserter(matches));
            }
        };
        std::visit(match_incoming_order, order_variant);
    }
    return matches;
}

void
BaseMatchingCycle::handle_matches_(std::vector<common::match> matches)
{
    std::vector<common::position> ob_updates{};

    for (auto [symbol, info] : tickers_) {
        auto tmp = info.get_orderbook().get_and_reset_updates();
        std::ranges::copy(tmp, std::back_inserter(ob_updates));
    }

    if (ob_updates.empty() && matches.empty())
        return;

    common::tick_update updates{ob_updates, matches};
    auto update = glz::write_json(updates);
    if (!update.has_value()) [[unlikely]] {
        throw std::runtime_error(glz::format_error(update.error()));
    }

    std::for_each(
        traders_.begin(), traders_.end(),
        [&message = *update](GenericTrader& trader) { trader.send_message(message); }
    );
}

} // namespace nutc::exchange
