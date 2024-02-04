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
#include "config.h"

#include <random>

namespace nutc {

namespace stochastic {

enum class Signedness { Negative = -1, DoesntMatter = 0, Positive = 1 };

class BrownianMotion {
    std::mt19937 random_number_generator_; // It's pretty obvious what this does
    double cur_value_; // Current value, used to generate next one
    double probability_ = 0.95; // probability of no market event

    // Control the actual ticking, whereby market events are slowed over many ticks
    size_t ticker_ = 0;
    Signedness signedness = Signedness::DoesntMatter;

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
        set_seed(seed);
    }

    // Generates and returns the next price based on previous prices
    void generate_next_price();

    [[nodiscard]] double
    get_current_price() const
    {
        return cur_value_;
    }

    // Force set the current price
    void
    set_price(double new_price)
    {
        cur_value_ = new_price;
    }

    // Force set the probability of market event
    void
    set_probability(double new_probability)
    {
        probability_ = new_probability;
    }

    // Force set the seed to something else
    void
    set_seed(unsigned int new_seed)
    {
        random_number_generator_ = std::mt19937(new_seed);
    }

private:
    // Generates and returns the change in price, i.e. dp/dt
    [[nodiscard]] double
    generate_change_in_price_(double mean, double stdev, Signedness sign);
};

} // namespace stochastic
} // namespace nutc
