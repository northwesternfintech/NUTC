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
            -cur_value_ / BrownianMotionParameters::TICK_SKEW_SCALE, BrownianMotionParameters::EVENT_SIZE_STDEV, signedness_
        );
        return cur_value_;
    }

    // Not ticking, so we need a random number
    std::uniform_real_distribution<double> zero_to_one_nd(0.0, 1.0);
    double random_number = zero_to_one_nd(random_number_generator_);

    // Create a market event
    if (random_number >= BrownianMotionParameters::ONE_MINUS_EVENT_PROBABILITY) {
        std::normal_distribution<double> distribution(
            BrownianMotionParameters::EVENT_SIZE_MEAN, BrownianMotionParameters::EVENT_SIZE_STDEV
        );
        ticker_ = static_cast<size_t>(abs(distribution(random_number_generator_)));

        // Whether to tick up or tick down
        bool ticking_up = distribution(random_number_generator_) > 0.5;
        signedness_ = ticking_up ? Signedness::Positive : Signedness::Negative;

        // Generate new price
        cur_value_ += generate_change_in_price_(
            -cur_value_ / BrownianMotionParameters::TICK_SKEW_SCALE, BrownianMotionParameters::EVENT_SIZE_STDEV,
            Signedness::DoesntMatter
        );
        return cur_value_;
    }
    else {
        cur_value_ += generate_change_in_price_(
            -cur_value_ / BrownianMotionParameters::TICK_SKEW_SCALE, BrownianMotionParameters::EVENT_SIZE_STDEV / BrownianMotionParameters::EVENT_DEVIATION_FACTOR,
            Signedness::DoesntMatter
        );
        return cur_value_;
    }
}

} // namespace stochastic
} // namespace nutc
