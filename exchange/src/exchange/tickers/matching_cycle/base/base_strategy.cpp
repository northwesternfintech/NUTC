#include "base_strategy.hpp"

namespace nutc {
namespace matching {
void
BaseMatchingCycle::before_cycle_(uint64_t new_tick)
{
    for (auto& [symbol, ticker_info] : tickers_) {
        generate_bot_orders_(ticker_info.bot_container, ticker_info.orderbook);
        expire_old_orders_(ticker_info.orderbook, new_tick);
    }
}

std::vector<stored_order>
BaseMatchingCycle::collect_orders(uint64_t new_tick)
{
    std::vector<stored_order> orders;
    for (const auto& trader : traders_) {
        auto incoming_orders = trader->read_orders();
        for (auto& order : incoming_orders) {
            orders.emplace_back(
                trader, order.side, order.ticker, order.quantity, order.price, new_tick
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
        if (order.price < 0 || order.quantity <= 0)
            continue;

        auto it = tickers_.find(order.ticker);
        if (it == tickers_.end())
            continue;

        auto tmp = it->second.engine.match_order(it->second.orderbook, order);
        std::ranges::move(tmp, std::back_inserter(matches));
    }
    return matches;
}

void
BaseMatchingCycle::handle_matches_(std::vector<stored_match> matches)
{
    std::vector<ob_update> ob_updates{};

    for (auto& [ticker, info] : tickers_) {
        auto tmp = info.level_update_generator_->get_updates(ticker);
        std::ranges::move(tmp, std::back_inserter(ob_updates));
    }

    std::vector<messages::match> glz_matches{};
    glz_matches.reserve(matches.size());
    for (auto& match : matches) {
        glz_matches.emplace_back(
            match.ticker, match.side, match.price, match.quantity,
            match.buyer->get_id(), match.seller->get_id(), match.buyer->get_capital(),
            match.seller->get_capital()
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
    for (auto& [ticker, info] : tickers_) {
        info.level_update_generator_->reset();
    }
}

void
BaseMatchingCycle::generate_bot_orders_(
    bots::BotContainer& bot_container, const OrderBook& orderbook
)
{
    bot_container.generate_orders(orderbook.get_midprice());
}
} // namespace matching
} // namespace nutc
