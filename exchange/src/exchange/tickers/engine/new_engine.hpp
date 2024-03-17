#pragma once

#include "exchange/traders/trader_manager.hpp"
#include "order_container.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "shared/util.hpp"

using MarketOrder = nutc::messages::MarketOrder;
using ObUpdate = nutc::messages::ObUpdate;
using Match = nutc::messages::Match;
using SIDE = nutc::messages::SIDE;

namespace nutc {
namespace matching {

class NewEngine {
    OrderContainer order_container_;
    uint64_t current_tick_ = 0;

public:
    std::vector<Match>
    match_order(const MarketOrder& order)
    {
        auto trader =
            manager::ClientManager::get_instance().get_trader(order.client_id);
        order_container_.add_order(StoredOrder{
            trader, order.side, order.ticker, order.quantity, order.price, current_tick_
        });
        return attempt_matches_();
    }

    std::vector<StoredOrder>
    expire_old_orders(uint64_t new_tick)
    {
        assert(new_tick == current_tick_ + 1);
        auto orders = order_container_.expire_orders(current_tick_);
        current_tick_ = new_tick;
        return orders;
    }

private:
    bool
    handle_order_failure_(const StoredOrder& buyer, const StoredOrder& seller)
    {
        double quantity = std::min(buyer.quantity, seller.quantity);
        double price =
            buyer.order_index < seller.order_index ? buyer.price : seller.price;
        if (buyer.trader->get_capital() < price * quantity) {
            order_container_.remove_order(buyer.order_index);
            return true;
        }
        if (seller.trader->get_holdings(buyer.ticker) < buyer.quantity) {
            order_container_.remove_order(seller.order_index);
            return true;
        }
        return false;
    }

    static Match
    build_match(const StoredOrder& buyer, const StoredOrder& seller)
    {
        double quantity = std::min(buyer.quantity, seller.quantity);
        double price =
            buyer.order_index < seller.order_index ? buyer.price : seller.price;
        SIDE aggressive_side =
            buyer.order_index < seller.order_index ? seller.side : buyer.side;
        const std::string& buyer_id = buyer.trader->get_id();
        const std::string& seller_id = seller.trader->get_id();
        return Match{buyer.ticker, aggressive_side, price,
                     quantity,     buyer_id,        seller_id};
    }

    std::vector<Match>
    attempt_matches_()
    {
        std::vector<Match> matches;
        while (order_container_.can_match_orders()) {
            const StoredOrder& highest_bid = order_container_.top_order(SIDE::BUY);
            const StoredOrder& cheapest_ask = order_container_.top_order(SIDE::SELL);
            assert(highest_bid.price >= cheapest_ask.price);
            assert(highest_bid.ticker == cheapest_ask.ticker);

            if (handle_order_failure_(highest_bid, cheapest_ask))
                continue;

            double quantity = std::min(highest_bid.quantity, cheapest_ask.quantity);

            matches.push_back(build_match(highest_bid, cheapest_ask));
            order_container_.modify_order_quantity(highest_bid.order_index, -quantity);
            order_container_.modify_order_quantity(cheapest_ask.order_index, -quantity);
        }
        return matches;
    }
};

} // namespace matching
} // namespace nutc
