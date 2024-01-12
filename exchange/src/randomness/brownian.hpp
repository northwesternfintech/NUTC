/**
 * @file brownian.hpp
 * @author Andrew Li (andrewli@u.northwestern.edu)
 * @brief Brownian motion randomness to simulate market chaos
 * @version 0.1
 * @date 2024-01-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <vector>

namespace nutc {

namespace brownian {

class BrownianMotion {
    std::vector<float> previous;
    int seed;

public:
    // Constructor for BrownianMotion, takes a seed
    BrownianMotion(const int seed) : seed(seed) {};

    // Generates and returns the next price based on previous prices
    float
    generate_next_price();

    // Force push a new price into the stack to affect
    // the rng used to generate the next float
    void
    inject_price(
        float new_price
    )
    {
        this->previous.push_back(new_price);
    }

    // Force change the seed to something else
    void
    force_set_seed(
        int new_seed
    )
    {
        this->seed = new_seed;
    }
};

} // namespace brownian
} // namespace nutc
