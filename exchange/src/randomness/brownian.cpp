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

const double BROWNIAN_MOTION_DEVIATION = 0.4;

namespace nutc {
namespace stochastic {

double
BrownianMotion::generate_next_price()
{
    double current_price = this->cur_value;
    std::normal_distribution<double> distribution(
        -cur_value / 1000, BROWNIAN_MOTION_DEVIATION
    );

    double delta_current_price = distribution(random_number_generator);
    double new_price = current_price + delta_current_price;

    cur_value = new_price;
    return new_price;
}

} // namespace stochastic
} // namespace nutc
