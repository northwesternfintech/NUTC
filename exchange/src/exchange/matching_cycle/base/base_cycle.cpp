#include "base_cycle.hpp"

#include "common/messages_exchange_to_wrapper.hpp"
#include "common/messages_wrapper_to_exchange.hpp"

#include <variant>

namespace nutc::exchange {

void
BaseMatchingCycle::before_cycle_(uint64_t)
{
    for (auto [symbol, ticker_info] : tickers_) {
        for (auto& bot_container : ticker_info.bot_containers) {
            bot_container.generate_orders(ticker_info.limit_orderbook.get_midprice());
        }
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
            [&](auto& v) { return std::visit(get_tagged_order, v); }
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
        auto match_order = [&]<typename OrderT>(OrderT& order) {
            // TODO: expose correct thing yk
            auto& ticker_info = tickers_[order.ticker];
            auto& orderbook = ticker_info.limit_orderbook;
            if constexpr (std::is_same_v<OrderT, common::cancel_order>) {
                orderbook.remove_order(order.order_id);
            }
            else {
                if (order.quantity <= 0.0)
                    return;
                auto& engine = ticker_info.engine;
                auto tmp = engine.match_order(order, orderbook);
                std::copy(tmp.begin(), tmp.end(), std::back_inserter(matches));
            }
        };
        std::visit(match_order, order_variant);
    }
    return matches;
}

void
BaseMatchingCycle::handle_matches_(std::vector<common::match> matches)
{
    std::vector<common::position> ob_updates{};

    for (auto [symbol, info] : tickers_) {
        auto tmp = info.limit_orderbook.get_update_generator().get_updates();
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

void
BaseMatchingCycle::post_cycle_(uint64_t)
{
    for (auto [_, info] : tickers_) {
        info.limit_orderbook.get_update_generator().reset();
    }
}

} // namespace nutc::exchange
