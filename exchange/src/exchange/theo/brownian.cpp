// Brownian motion stuff
#define BROWNIAN_MOTION_MEAN_SIZE_EVENT  15
#define BROWNIAN_MOTION_STDEV_EVENT_SIZE 5
#define BROWNIAN_MOTION_DEVIATION        .1
#define SKEW_SCALE                                                                     \
    2000 // Factor by which the market wants to return to baseline (larger = less)
#define SKEW_FACTOR 4 // How much more market events skew than normal

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

namespace nutc {
namespace stochastic {

double
BrownianMotion::generate_change_in_price_(double mean, double stdev, Signedness sign)
{
    std::normal_distribution<double> distribution(mean, stdev);

    if (sign == Signedness::DoesntMatter) {
        return distribution(random_number_generator_);
    }
    else {
        short multiplier = static_cast<short>(sign);
        return multiplier * abs(distribution(random_number_generator_));
    }
}

double
BrownianMotion::generate_next_price()
{
    if (ticker_ > 0) {
        ticker_--;
        cur_value_ += generate_change_in_price_(
            -cur_value_ / SKEW_SCALE, BROWNIAN_MOTION_DEVIATION, signedness_
        );
        return cur_value_;
    }

    // Not ticking, so we need a random number
    std::uniform_real_distribution<double> zero_to_one_nd(0.0, 1.0);
    double random_number = zero_to_one_nd(random_number_generator_);

    if (random_number >= probability_) {
        std::normal_distribution<double> distribution(
            BROWNIAN_MOTION_MEAN_SIZE_EVENT, BROWNIAN_MOTION_STDEV_EVENT_SIZE
        );
        ticker_ = static_cast<size_t>(abs(distribution(random_number_generator_)));

        // Whether to tick up or tick down
        bool ticking_up = distribution(random_number_generator_) > 0.5;
        signedness_ = ticking_up ? Signedness::Positive : Signedness::Negative;

        // Generate new price
        cur_value_ += generate_change_in_price_(
            -cur_value_ / SKEW_SCALE, BROWNIAN_MOTION_DEVIATION,
            Signedness::DoesntMatter
        );
        return cur_value_;
    }
    else {
        cur_value_ += generate_change_in_price_(
            -cur_value_ / SKEW_SCALE, BROWNIAN_MOTION_DEVIATION / SKEW_FACTOR,
            Signedness::DoesntMatter
        );
        return cur_value_;
    }
}

} // namespace stochastic
} // namespace nutc
