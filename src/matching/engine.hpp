#pragma once
//
// Created by echav on 9/4/2023.
//

#include "match.hpp"
#include "order.hpp"

#include <chrono>

#include <vector>

namespace nutc {
namespace matching {

class Engine {
public: // we will need to add all communication machinery in, this will just expose
    std::vector<Order> bids;
    std::vector<Order> asks;

    Engine(); // con
    void add_order(Order order);
    std::vector<Match> match();
};
} // namespace matching
} // namespace nutc
