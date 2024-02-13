/**
 * @file brownian.cpp
 * @author Andrew Li (andrewli@u.northwestern.edu)
 * @brief Brownian motion randomness to simulate market chaos
 * @version 0.1
 * @date 2024-01-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "brownian.hpp"

namespace nutc {
namespace stochastic {

double
BrownianMotion::generate_next_price()
{
    if (ticker_ > 0) {
        ticker_--;
        int sign = ticking_up_ ? 1 : -1;
        double new_price = cur_value_;

        // Generate change in price
        std::normal_distribution<double> distribution(
            -cur_value_ / skew_scale_, BROWNIAN_MOTION_DEVIATION
        );
        double delta_current_price = distribution(random_number_generator_);
        double new_price = new_price + delta_current_price;
        cur_value_ = new_price;
        return new_price;
    }

    // Not ticking, so we need a random number
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    double random_number = distribution(random_number_generator_);

    if (random_number >= probability_) {
        std::normal_distribution<double> distribution(
            BROWNIAN_MOTION_MEAN_SIZE_EVENT, BROWNIAN_MOTION_STDEV_EVENT_SIZE
        );
        ticker_ = (int)abs(distribution(random_number_generator_));

        // Whether to tick up or tick down
        ticking_up_ = distribution(random_number_generator_) > 0.5;

        // Generate change in price
        std::normal_distribution<double> distribution(
            -cur_value_ / skew_scale_, BROWNIAN_MOTION_DEVIATION
        );
        double delta_current_price = distribution(random_number_generator_);
        double new_price = new_price + delta_current_price;
        cur_value_ = new_price;
        return new_price;
    }
    else {
        // Generate change in price
        std::normal_distribution<double> distribution(
            -cur_value_ / skew_scale_, BROWNIAN_MOTION_DEVIATION / SKEW_FACTOR
        );
        double delta_current_price = distribution(random_number_generator_);
        double new_price = new_price + delta_current_price;
        cur_value_ = new_price;
        return new_price;
    }
}

} // namespace stochastic
} // namespace nutc
