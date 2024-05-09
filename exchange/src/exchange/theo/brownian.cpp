#define BROWNIAN_MOTION_MAGNITUDE_STDEV 0.022 //.22
#define MARKET_EVENT_MAGNITUDE_STDEV    0.03 //.03
#define MARKET_EVENT_PROBABILITY        0.02 //.02
#define MARKET_DURATION_MEAN            15
#define MARKET_DURATION_STDEV           15

#include "brownian.hpp"

#include "exchange/logging.hpp"

namespace nutc {
namespace stochastic {

double
BrownianMotion::generate_norm_(double mean, double stdev, Signedness sign)
{
    std::normal_distribution<double> distribution(mean, stdev);

    double norm = distribution(random_number_generator_);

    if (sign == Signedness::Either) {
        return norm;
    }

    auto sign_multiplier = static_cast<int16_t>(sign);
    return (sign_multiplier * fabs(norm));
}

double
BrownianMotion::generate_uniform_(double low, double high)
{
    std::uniform_real_distribution<double> zero_to_one_nd(low, high);
    double random_number = zero_to_one_nd(random_number_generator_);
    return random_number;
}

bool
BrownianMotion::generate_bool_(double probability_of_true)
{
    double random_number =
        generate_uniform_(-probability_of_true, 1 - probability_of_true);
    static int count_false = 0;
    static int count_true = 0;
    if (random_number > 0)
        count_true++;
    else
        count_false++;
    log_i(main, "{} - {}", count_false, count_true);
    return random_number > 0;
}

double
BrownianMotion::generate_brownian_motion_(double stdev, Signedness direction)
{
    return generate_norm_(0, stdev, direction) * std::pow((cur_magnitude_ / 100), .1);
}

void
BrownianMotion::config_new_market_event_()
{
    auto val = generate_norm_(
        MARKET_DURATION_MEAN, MARKET_DURATION_STDEV, Signedness::Positive
    );
    event_ticks_remaining_ = static_cast<int>(val);

    bool ticking_up = generate_bool_(0.5); // 50% true
    event_direction_ = ticking_up ? Signedness::Positive : Signedness::Negative;
}

double
BrownianMotion::generate_market_tick_()
{
    // Add one where the direction is locked, and then one where direction is not
    // locked to *trend* in one direction but not force directionality.
    double delta =
        generate_brownian_motion_(MARKET_EVENT_MAGNITUDE_STDEV, event_direction_);
    delta += generate_brownian_motion_(MARKET_EVENT_MAGNITUDE_STDEV);

    return delta;
}

double
BrownianMotion::generate_nonmarket_tick_()
{
    double delta = generate_brownian_motion_(BROWNIAN_MOTION_MAGNITUDE_STDEV);
    return delta;
}

bool
BrownianMotion::should_start_new_market_event_()
{
    double random_number = generate_uniform_(0.0, 1.0);
    return random_number <= MARKET_EVENT_PROBABILITY;
}

double
BrownianMotion::generate_next_magnitude()
{
    bool market_event_ongoing = event_ticks_remaining_ > 0;

    if (!market_event_ongoing && should_start_new_market_event_()) {
        config_new_market_event_();
        market_event_ongoing = true;
    }

    if (market_event_ongoing) {
        event_ticks_remaining_--;
        cur_magnitude_ += generate_market_tick_();
        return fabs(cur_magnitude_);
    }

    // Handle as a normal tick
    cur_magnitude_ += generate_nonmarket_tick_();
    return fabs(cur_magnitude_);
}

} // namespace stochastic
} // namespace nutc
