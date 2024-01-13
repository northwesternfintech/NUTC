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

#include <random>
#include <vector>

namespace nutc {

namespace brownian {

const double BROWNIAN_MOTION_DEVIATION = 0.4;

class BrownianMotion {
    int seed;
    double cur_value;
    double deviation;

    std::mt19937 random_number_generator;

public:
    // dConstructor for BrownianMotion, takes nothing
    BrownianMotion() : deviation(BROWNIAN_MOTION_DEVIATION) {
        std::random_device rd;
        random_number_generator = std::mt19937(rd());
    };

    // Constructor for BrownianMotion, takes a seed
    BrownianMotion(const int seed) : seed(seed), deviation(BROWNIAN_MOTION_DEVIATION) {
        random_number_generator = std::mt19937(seed);
    };

    // Constructor for BrownianMotion, takes a seed and initial value
    BrownianMotion(const int seed, const double initial_value) : seed(seed), cur_value(initial_value), deviation(BROWNIAN_MOTION_DEVIATION) {
        random_number_generator = std::mt19937(seed);
    };

    // Generates and returns the next price based on previous prices
    double
    generate_next_price();

    // Force set the current price
    void
    inject_price(
        double new_price
    )
    {
        cur_value = new_price;
    }

    // Force set the seed to something else
    void
    force_set_seed(
        int new_seed
    )
    {
        seed = new_seed;
    }

};

} // namespace brownian
} // namespace nutc
