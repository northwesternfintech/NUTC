#include "base_cycle.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"

namespace nutc {
namespace matching {
void
BaseMatchingCycle::before_cycle_(uint64_t)
{
    for (auto& [ticker_info, _, symbol] : tickers_) {
        for (auto& bot_container : ticker_info.bot_containers) {
            bot_container.generate_orders(ticker_info.orderbook.get_midprice());
        }
    }
}

std::vector<stored_order>
BaseMatchingCycle::collect_orders(uint64_t)
{
    std::vector<stored_order> orders;

    auto collect_orders = [&orders](traders::GenericTrader& trader) {
        auto handle_order = [&orders, &trader]<typename ordered>(const ordered& order) {
            if constexpr (std::is_same_v<ordered, messages::limit_order>) {
                orders.emplace_back(
                    trader, order.position.ticker, order.position.side,
                    order.position.quantity, order.position.price, order.ioc
                );
            }
            else if constexpr (std::is_same_v<ordered, messages::market_order>) {
                orders.emplace_back(
                    trader, order.ticker, order.side, order.quantity,
                    order.side == util::Side::buy ? 1000.0 : 0.0, true
                );
            }
        };

        auto incoming_orders = trader.read_orders();
        std::for_each(
            incoming_orders.begin(), incoming_orders.end(),
            [&handle_order](const auto& order_variant) {
                std::visit(handle_order, order_variant);
            }
        );
    };

    std::for_each(traders_.begin(), traders_.end(), collect_orders);

    return orders;
}

std::vector<stored_match>
BaseMatchingCycle::match_orders_(std::vector<stored_order> orders)
{
    std::vector<stored_match> matches{};
    for (auto& order : orders) {
        if (order.position.price < 0.0 || order.position.quantity <= 0)
            continue;

        auto it = tickers_.find(order.position.ticker);
        if (it == tickers_.end())
            continue;

        it->second.orderbook.add_order(order);
        auto tmp = it->second.engine.match_orders(it->second.orderbook);
        std::ranges::move(tmp, std::back_inserter(matches));
    }
    return matches;
}

void
BaseMatchingCycle::handle_matches_(std::vector<stored_match> matches)
{
    std::vector<util::position> ob_updates{};

    for (auto& [info, _, ticker] : tickers_) {
        auto tmp = info.orderbook.get_update_generator().get_updates(ticker);
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
    std::string update_str = glz::write_json(updates);

    std::for_each(
        traders_.begin(), traders_.end(),
        [&update_str](traders::GenericTrader& trader) {
            trader.send_message(update_str);
        }
    );
}

void
BaseMatchingCycle::post_cycle_(uint64_t)
{
    for (auto& [info, _, ticker] : tickers_) {
        info.orderbook.remove_ioc_orders();
        info.orderbook.get_update_generator().reset();
    }
}

} // namespace matching
} // namespace nutc
