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
    double cur_value_;
    std::mt19937 random_number_generator_;

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

    // Constructor for BrownianMotion, takes a seed and initial value
    explicit BrownianMotion(const unsigned int seed, const double initial_value) :
        cur_value_(initial_value)
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
