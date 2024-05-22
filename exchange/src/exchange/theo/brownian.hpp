/**
 * @file brownian.hpp
 * @author Andrew Li (andrewli@u.northwestern.edu)
 * @brief Brownian motion theo to simulate market chaos
 * @version 0.1
 * @date 2024-01-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "exchange/config/static/config.h"

#include <random>

namespace nutc {

namespace stochastic {

enum class Signedness { Negative = -1, Either = 0, Positive = 1 };

class BrownianMotion {
    std::minstd_rand0 random_number_generator_;
    double cur_magnitude_;

    size_t event_ticks_remaining_ = 0;
    Signedness event_direction_ = Signedness::Either;

public:
    [[nodiscard]] double
    get_magnitude() const
    {
        return cur_magnitude_;
    }

    // Default constructor for BrownianMotion, takes nothing
    explicit BrownianMotion() : cur_magnitude_(0)
    {
        std::random_device rd;
        random_number_generator_ = std::minstd_rand0(rd());
    }

    // Constructor for BrownianMotion, takes a seed
    explicit BrownianMotion(const unsigned int seed) : cur_magnitude_(0)
    {
        set_seed(seed);
    }

    // Constructor for BrownianMotion with initial price
    explicit BrownianMotion(const double starting_magnitude) :
        cur_magnitude_(starting_magnitude)
    {
        std::random_device rd;
        random_number_generator_ = std::minstd_rand0(rd());
    }

    // Generates and returns the next magnitude based on previous magnitudes
    double generate_next_magnitude();

    // Force set the current magnitude
    void
    set_magnitude(double new_magnitude)
    {
        cur_magnitude_ = new_magnitude;
    }

    // Force set the seed to something else
    void
    set_seed(unsigned int new_seed)
    {
        random_number_generator_ = std::minstd_rand0(new_seed);
    }

private:
    double
    generate_norm_(double mean, double stdev, Signedness sign = Signedness::Either);
    double generate_uniform_(double low, double high);
    bool generate_bool_(double probability_of_true);

    double
    generate_brownian_motion_(double mean, Signedness direction = Signedness::Either);
    double generate_market_tick_();
    double generate_nonmarket_tick_();
    bool should_start_new_market_event_();
    void config_new_market_event_();
};

} // namespace stochastic
} // namespace nutc
