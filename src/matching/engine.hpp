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
using ObUpdate = nutc::messages::ObUpdate;
using Match = nutc::messages::Match;

namespace nutc {
namespace matching {

class Engine {
public: // we will need to add all communication machinery in, this will just expose
    std::priority_queue<MarketOrder> bids;
    std::priority_queue<MarketOrder> asks;

    Engine(); // con
    std::optional<std::pair<const std::vector<Match>, const std::vector<ObUpdate>>>
    add_order_and_match(MarketOrder aggressive_order);

private:
    void add_order(MarketOrder aggressive_order);
    std::pair<std::vector<Match>, std::vector<ObUpdate>>
    match_buy_order(MarketOrder aggressive_order);
    std::pair<std::vector<Match>, std::vector<ObUpdate>>
    match_sell_order(MarketOrder aggressive_order);
    ObUpdate create_ob_update(const MarketOrder& order, float quantity);
};
} // namespace matching
} // namespace nutc
