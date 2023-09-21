#pragma once
//
// Created by echav on 9/4/2023.
//

#include "logging.hpp"
#include "messages.hpp"

#include <chrono>

#include <optional>
#include <queue>
#include <vector>

using MarketOrder = nutc::messages::MarketOrder;
using Match = nutc::messages::Match;

namespace nutc {
namespace matching {

class Engine {
public: // we will need to add all communication machinery in, this will just expose
    std::priority_queue<MarketOrder> bids;
    std::priority_queue<MarketOrder> asks;

    Engine(); // con
    const std::optional<const std::vector<Match>>
    add_order_and_match(MarketOrder aggressive_order);

private:
    void add_order(MarketOrder aggressive_order);
    std::vector<Match> match_buy_order(MarketOrder aggressive_order);
    std::vector<Match> match_sell_order(MarketOrder aggressive_order);
};
} // namespace matching
} // namespace nutc
