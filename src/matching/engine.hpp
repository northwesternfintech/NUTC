#pragma once
//
// Created by echav on 9/4/2023.
//

#include "order.hpp"

#include <vector>

namespace nuft {
namespace matching {

class Engine {
public: // we will need to add all communication machinery in, this will just expose
    std::vector<Order> bids;
    std::vector<Order> asks;

    Engine(); // con
    void add_order(Order order);
    void match();
};
} // namespace matching
} // namespace nuft
