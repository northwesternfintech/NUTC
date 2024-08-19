#pragma once

#include "types/position.hpp"
#include "types/ticker.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>

namespace nutc {
namespace messages {

struct init_message {
    bool successful_startup;
};

struct limit_order {
    util::position position;
    bool ioc;

    bool operator==(const limit_order& other) const = default;

    limit_order(
        util::Side side, util::Ticker ticker, double quantity,
        util::decimal_price price, bool ioc = false
    ) :
        position{side, ticker, quantity, price},
        ioc(ioc)
    {}

    limit_order(const util::position& position, bool ioc = false) :
        position(position), ioc(ioc)
    {}

    limit_order(util::Side side, util::Ticker ticker, double quantity) :
        position{side, ticker, quantity, market_order_price(side)}, ioc{true}
    {}

    limit_order() = default;

private:
    static util::decimal_price
    market_order_price(util::Side side)
    {
        if (side == util::Side::buy) {
            return std::numeric_limits<util::decimal_price>::max();
        }
        else {
            return std::numeric_limits<util::decimal_price>::min();
        }
    }
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::limit_order> {
    using t = nutc::messages::limit_order;
    static constexpr auto value = object(&t::position, &t::ioc);
};

/// \cond
template <>
struct glz::meta<nutc::messages::init_message> {
    using t = nutc::messages::init_message;
    static constexpr auto value = object(&t::successful_startup);
};
