#pragma once

#include "exchange/logging.hpp"
#include "exchange/randomness/brownian.hpp"
#include "exchange/tick_manager/tick_observer.hpp"

#include <iostream>

namespace nutc {
namespace stochastic {

// Singleton is probably a dumb way to do this but it's fine for now
class BrownianTickListener : stochastic::BrownianMotion, public ticks::TickObserver {
public:
    void
    on_tick() override
    {
        generate_next_price();
        double price = get_current_price();
        log_i(main, "Current price: {}", std::to_string(price));
    }

    [[nodiscard]] double
    get_current_price() const
    {
        return BrownianMotion::get_current_price();
    }

private:
    explicit BrownianTickListener(const double initial_value) :
        stochastic::BrownianMotion(initial_value)
    {}

public:
    static BrownianTickListener&
    get_instance(double initial_value)
    {
        static BrownianTickListener instance(initial_value);
        return instance;
    }

    BrownianTickListener(BrownianTickListener const&) = delete;
    void operator=(BrownianTickListener const&) = delete;
    BrownianTickListener(BrownianTickListener&&) = delete;
    void operator=(BrownianTickListener&&) = delete;
};
} // namespace stochastic
} // namespace nutc
