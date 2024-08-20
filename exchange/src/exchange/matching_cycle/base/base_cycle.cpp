#include "base_cycle.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"

#include <variant>

namespace nutc {
namespace matching {
void
BaseMatchingCycle::before_cycle_(uint64_t)
{
    for (auto& [ticker_info, _, symbol] : tickers_) {
        for (auto& bot_container : ticker_info.bot_containers) {
            bot_container.generate_orders(ticker_info.limit_orderbook.get_midprice());
        }
    }
}

auto
BaseMatchingCycle::collect_orders(uint64_t) -> std::vector<OrderVariant>
{
    std::vector<OrderVariant> orders;

    auto collect_orders = [&orders](traders::GenericTrader& trader) {
        auto message_queue = trader.read_orders();

        auto get_tagged_order =
            [&trader]<typename OrderT>(const OrderT& order
            ) -> std::variant<tagged_limit_order, tagged_market_order> {
            if constexpr (std::is_same_v<OrderT, messages::timed_init_message>) {
                throw std::runtime_error("Unexpected initialization message");
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

std::vector<stored_match>
BaseMatchingCycle::match_orders_(std::vector<OrderVariant> orders)
{
    std::vector<stored_match> matches;

    for (const auto& order_variant : orders) {
        auto match_order = [&]<typename T>(const T& order) {
            if (order.quantity <= 0)
                return;

            auto it = tickers_.find(order.ticker);
            if (it == tickers_.end())
                return;

            if constexpr (std::is_same_v<T, tagged_limit_order>) {
                it->second.limit_orderbook.add_order(order);
                auto tmp = it->second.engine.match_orders(it->second.limit_orderbook);
                std::copy(tmp.begin(), tmp.end(), std::back_inserter(matches));
            }
            else if constexpr (std::is_same_v<T, tagged_market_order>) {
                auto order2 = tagged_limit_order{
                    *order.trader,
                    messages::limit_order{
                                          order.ticker, order.side, order.quantity,
                                          order.side == util::Side::buy ? 1000.0 : 0.0, true
                    }
                };
                it->second.limit_orderbook.add_order(order2);
                auto tmp = it->second.engine.match_orders(it->second.limit_orderbook);
                std::copy(tmp.begin(), tmp.end(), std::back_inserter(matches));
            }
            else {
                static_assert(std::is_same_v<T, tagged_market_order>);
            }
        };
        std::visit(match_order, order_variant);
    }
    return matches;
}

void
BaseMatchingCycle::handle_matches_(std::vector<stored_match> matches)
{
    std::vector<util::position> ob_updates{};

    for (auto& [info, _, ticker] : tickers_) {
        auto tmp = info.limit_orderbook.get_update_generator().get_updates(ticker);
        std::ranges::copy(tmp, std::back_inserter(ob_updates));
    }

    std::vector<messages::match> glz_matches{};
    glz_matches.reserve(matches.size());
    for (auto& match : matches) {
        glz_matches.emplace_back(
            match.position, match.buyer.get_id(), match.seller.get_id(),
            match.buyer.get_capital(), match.seller.get_capital()
        );
    }

    if (ob_updates.empty() && glz_matches.empty())
        return;

    messages::tick_update updates{ob_updates, glz_matches};
    auto update = glz::write_json(updates);
    if (!update.has_value()) [[unlikely]] {
        throw std::runtime_error(glz::format_error(update.error()));
    }

    std::for_each(
        traders_.begin(), traders_.end(),
        [&message = *update](traders::GenericTrader& trader) {
            trader.send_message(message);
        }
    );
}

void
BaseMatchingCycle::post_cycle_(uint64_t)
{
    for (auto& [info, _, ticker] : tickers_) {
        info.limit_orderbook.remove_ioc_orders();
        info.limit_orderbook.get_update_generator().reset();
    }
}

} // namespace matching
} // namespace nutc
