#pragma once
//
// Created by echav on 9/4/2023.
//

#include "match.hpp"
#include "messages.hpp"

#include <chrono>

#include <vector>

using MarketOrder = nutc::messages::MarketOrder;

namespace nutc {
namespace matching {

class Engine {
public: // we will need to add all communication machinery in, this will just expose
    std::vector<MarketOrder> bids;
    std::vector<MarketOrder> asks;

    Engine(); // con
    void add_order(MarketOrder order);
    std::vector<Match> match();
};
} // namespace matching
} // namespace nutc
