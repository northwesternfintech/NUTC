#include "base_strategy.hpp"

namespace nutc {
namespace matching {
void
BaseMatchingCycle::before_cycle_(uint64_t)
{
    for (auto& [symbol, ticker_info] : tickers_) {
        auto& bot_container = ticker_info.bot_container;
        auto& orderbook = ticker_info.orderbook;

        bot_container.generate_orders(orderbook.get_midprice());
    }
}

std::vector<stored_order>
BaseMatchingCycle::collect_orders(uint64_t)
{
    std::vector<stored_order> orders;
    for (const std::shared_ptr<traders::GenericTrader>& trader : traders_) {
        auto incoming_orders = trader->read_orders();
        for (auto& order : incoming_orders) {
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
        if (order.position.price < decimal_price{0.0} || order.position.quantity <= 0)
            continue;

        auto it = tickers_.find(order.position.ticker);
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
    std::vector<util::position> ob_updates{};

    for (auto& [ticker, info] : tickers_) {
        auto tmp = info.level_update_generator_->get_updates(ticker);
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
    for (auto& [ticker, info] : tickers_) {
        info.orderbook.remove_ioc_orders();
        info.level_update_generator_->reset();
    }
}

} // namespace matching
} // namespace nutc
