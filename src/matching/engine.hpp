#pragma once

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
public: 
    std::priority_queue<MarketOrder> bids;
    std::priority_queue<MarketOrder> asks;
    std::pair<std::vector<Match>, std::vector<ObUpdate>>
    match_order(MarketOrder aggressive_order);

    Engine(); // con

private:
    void add_order(MarketOrder aggressive_order);
    ObUpdate create_ob_update(const MarketOrder& order, float quantity);
};
} // namespace matching
} // namespace nutc
