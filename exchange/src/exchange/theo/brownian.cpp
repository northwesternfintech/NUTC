/**
 * @file brownian.cpp
 * @author Andrew Li (andrewli@u.northwestern.edu)
 * @brief Brownian motion theo to simulate market chaos
 * @version 0.1
 * @date 2024-01-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "brownian.hpp"

constexpr double BROWNIAN_MOTION_DEVIATION = 0.4;

namespace nutc {
namespace stochastic {

double
BrownianMotion::generate_next_price()
{
    double current_price = cur_value_;
    std::normal_distribution<double> distribution(
        -cur_value_ / 1000, BROWNIAN_MOTION_DEVIATION
    );

    double delta_current_price = distribution(random_number_generator_);
    double new_price = current_price + delta_current_price;

    cur_value_ = new_price;
    return new_price;
}

} // namespace stochastic
} // namespace nutc
