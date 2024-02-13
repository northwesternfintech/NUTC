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

namespace nutc {

namespace stochastic {

class BrownianMotion {
    // It's pretty obvious what this does
    std::mt19937 random_number_generator_;

    // Current value, used to generate next one
    double cur_value_;

    // Control the size of market events and skewness of the distribution
    size_t mean_size_of_event;
    size_t stdev_of_event;
    size_t skew_scale;
    
    // Control the actual ticking, whereby market events are slowed over many ticks
    size_t ticker;
    bool ticking_up;

public:
    // Default constructor for BrownianMotion, takes nothing
    explicit BrownianMotion() : cur_value_(0)
    {
        std::random_device rd;
        random_number_generator_ = std::mt19937(rd());
    }

    // Constructor for BrownianMotion, takes a seed
    explicit BrownianMotion(const unsigned int seed) : cur_value_(0)
    {
        random_number_generator_ = std::mt19937(seed);
    }

    // Generates and returns the next price based on previous prices
    [[nodiscard]] double generate_next_price();

    // Force set the current price
    void
    set_price(double new_price)
    {
        cur_value_ = new_price;
    }

    // Force set the seed to something else
    void
    set_seed(unsigned int new_seed)
    {
        random_number_generator_ = std::mt19937(new_seed);
    }
};

} // namespace stochastic
} // namespace nutc
