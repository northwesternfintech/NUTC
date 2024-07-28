#include "base_strategy.hpp"

namespace nutc {
namespace matching {
void
BaseMatchingCycle::before_cycle_(uint64_t)
{
    for (auto& [ticker_info, _, symbol] : tickers_) {
        auto& orderbook = ticker_info.orderbook;
        for (auto& [type, bot_container] : ticker_info.bot_containers) {
            bot_container.generate_orders(orderbook.get_midprice());
        }
    }
}

std::vector<stored_order>
BaseMatchingCycle::collect_orders(uint64_t)
{
    std::vector<stored_order> orders;
    for (const std::shared_ptr<traders::GenericTrader>& trader : traders_) {
        auto incoming_orders = trader->read_orders();
        for (auto& order : incoming_orders) {
            // TODO: penalize?
            if (!order.position.price.valid_start_price())
                continue;

            orders.emplace_back(
                *trader, order.position.ticker, order.position.side,
                order.position.price, order.position.quantity, order.ioc
            );
        }
    }
    return orders;
}

std::vector<stored_match>
BaseMatchingCycle::match_orders_(std::vector<stored_order> orders)
{
    std::vector<stored_match> matches{};
    for (auto& order : orders) {
        if (order.position.price < util::decimal_price{0.0}
            || order.position.quantity <= 0)
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
    for (const auto& trader : traders_) {
        trader->send_message(update_str);
    }
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
