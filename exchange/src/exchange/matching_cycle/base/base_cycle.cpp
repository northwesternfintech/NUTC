#include "base_cycle.hpp"

#include "common/types/messages/messages_exchange_to_wrapper.hpp"
#include "common/types/messages/messages_wrapper_to_exchange.hpp"
#include "exchange/matching/engine.hpp"

#include <glaze/json/write.hpp>

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

std::vector<tagged_match>
BaseMatchingCycle::match_orders_(std::vector<OrderVariant> orders)
{
    std::vector<tagged_match> matches;

    auto match_incoming_order = [&]<typename OrderT>(OrderT& order) {
        auto& ticker_data = tickers_[order.ticker];
        auto& orderbook = ticker_data.get_orderbook();
        if constexpr (std::is_same_v<OrderT, common::cancel_order>) {
            orderbook.remove_order(order.order_id);
        }
        else {
            if (order.quantity <= 0.0)
                return;
            // TODO: delegate elsewhere
            if (order.quantity + order.trader->get_portfolio().get_open_bids()
                    + order.trader->get_portfolio().get_open_asks()
                > max_cumulative_order_volume_) {
                return;
            }
            auto tmp = match_order(order, orderbook, order_fee_);
            std::copy(tmp.begin(), tmp.end(), std::back_inserter(matches));
        }
    };

    for (OrderVariant& order_variant : orders) {
        std::visit(match_incoming_order, order_variant);
    }
    return matches;
}

std::vector<common::position>
BaseMatchingCycle::get_orderbook_updates_()
{
    std::vector<common::position> ob_updates;

    for (auto [symbol, info] : tickers_) {
        auto tmp = info.get_orderbook().get_and_reset_updates();
        std::ranges::copy(tmp, std::back_inserter(ob_updates));
    }
    return ob_updates;
}

std::vector<common::position>
BaseMatchingCycle::tagged_matches_to_positions(const std::vector<tagged_match>& matches)
{
    std::vector<common::position> untagged_matches(matches.size());
    std::transform(
        matches.begin(), matches.end(), untagged_matches.begin(),
        [](const tagged_match& match) { return match; }
    );
    return untagged_matches;
}

void
BaseMatchingCycle::send_account_updates(const std::vector<tagged_match>& matches)
{
    std::for_each(matches.begin(), matches.end(), [](const tagged_match& match) {
        common::position trade{match.ticker, match.side, match.quantity, match.price};
        common::account_update buyer_update{
            trade, match.buyer->get_portfolio().get_capital()
        };
        common::account_update seller_update{
            trade, match.seller->get_portfolio().get_capital()
        };
        auto buyer_update_str = glz::write_json(buyer_update);
        if (!buyer_update_str.has_value()) [[unlikely]] {
            throw std::runtime_error(glz::format_error(buyer_update_str.error()));
        }
        auto seller_update_str = glz::write_json(seller_update);
        if (!seller_update_str.has_value()) [[unlikely]] {
            throw std::runtime_error(glz::format_error(seller_update_str.error()));
        }
        match.buyer->send_message(buyer_update_str.value());
        match.seller->send_message(seller_update_str.value());
    });
}

void
BaseMatchingCycle::send_market_updates_(const std::vector<tagged_match>& matches)
{
    std::vector<common::position> orderbook_updates = get_orderbook_updates_();
    std::vector<common::position> trade_updates = tagged_matches_to_positions(matches);

    if (orderbook_updates.empty() && trade_updates.empty()) [[unlikely]]
        return;

    common::tick_update updates{orderbook_updates, trade_updates};
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
BaseMatchingCycle::handle_matches_(std::vector<tagged_match> matches)
{
    send_market_updates_(matches);
    send_account_updates(matches);
}

} // namespace nutc::exchange
