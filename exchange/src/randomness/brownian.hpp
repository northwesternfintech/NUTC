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

#define BROWNIAN_MOTION_MEAN_SIZE_EVENT  15
#define BROWNIAN_MOTION_STDEV_EVENT_SIZE 5
#define BROWNIAN_MOTION_DEVIATION        0.1
#define SKEW_SCALE                       20000
#define SKEW_FACTOR                      4

#include <random>

namespace nutc {

namespace stochastic {

class BrownianMotion {
    // It's pretty obvious what this does
    std::mt19937 random_number_generator_;

    // Current value, used to generate next one
    double cur_value_;

    // Control the size of market events
    size_t mean_size_of_event_ = BROWNIAN_MOTION_MEAN_SIZE_EVENT;
    size_t stdev_of_event_ = BROWNIAN_MOTION_STDEV_EVENT_SIZE;
    float probability_ = 0.9;

    // Control skewness of the distribution
    size_t skew_scale_ = SKEW_SCALE;

    // How much more market events skew than normal
    size_t skew_factor_ = SKEW_FACTOR;

    // Control the actual ticking, whereby market events are slowed over many ticks
    size_t ticker_ = 0;
    bool ticking_up_ = false;

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

    // Force set the probability of market event
    void
    set_probability(float new_probability)
    {
        probability_ = new_probability;
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
