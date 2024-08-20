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
            bot_container.generate_orders(ticker_info.orderbook.get_midprice());
        }
    }
}

auto
BaseMatchingCycle::collect_orders(uint64_t) -> OrderVectorPair
{
    OrderVectorPair orders;

    auto collect_orders = [&orders](traders::GenericTrader& trader) {
        auto [limit_orders, market_orders] = trader.read_orders();

        auto get_tagged_order = [&trader]<typename OrderT>(const OrderT& order) {
            return tagged_order<OrderT>{trader, order};
        };

        std::transform(
            limit_orders.begin(), limit_orders.end(), std::back_inserter(orders.first),
            get_tagged_order
        );
        std::transform(
            market_orders.begin(), market_orders.end(),
            std::back_inserter(orders.second), get_tagged_order
        );
    };

    std::for_each(traders_.begin(), traders_.end(), collect_orders);

    return orders;
}

std::vector<stored_match>
BaseMatchingCycle::match_orders_(OrderVectorPair orders)
{
    std::vector<stored_match> matches;

    for (const tagged_limit_order& order : orders.first) {
        if (order.position.price < 0.0 || order.position.quantity <= 0)
            continue;

        auto it = tickers_.find(order.position.ticker);
        if (it == tickers_.end())
            continue;

        it->second.orderbook.add_order(order);
        auto tmp = it->second.engine.match_orders(it->second.orderbook);
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(matches));
    }

    // TODO: change
    for (const tagged_market_order& order : orders.second) {
        if (order.quantity <= 0)
            continue;

        auto it = tickers_.find(order.ticker);
        if (it == tickers_.end())
            continue;

        tagged_limit_order lim_order{
            *order.trader,
            order.ticker,
            order.side,
            order.quantity,
            order.side == util::Side::buy ? 1000.0 : 0.0,
            true
        };

        it->second.orderbook.add_order(lim_order);
        auto tmp = it->second.engine.match_orders(it->second.orderbook);
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(matches));
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
        info.orderbook.remove_ioc_orders();
        info.orderbook.get_update_generator().reset();
    }
}

} // namespace matching
} // namespace nutc
