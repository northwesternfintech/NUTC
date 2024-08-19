#pragma once

#include "shared/types/decimal_price.hpp"
#include "shared/types/ticker.hpp"
#include "shared/util.hpp"

#include <glaze/glaze.hpp>

namespace nutc {
namespace util {
struct position {
    Side side;
    Ticker ticker;
    double quantity;
    util::decimal_price price;

    bool operator==(const position& other) const noexcept = default;

    position() = default;

    position(Side side, Ticker ticker, double quantity, util::decimal_price price) :
        side(side), ticker(ticker), quantity(quantity), price(price)
    {}
};

} // namespace util
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::util::position> {
    using t = nutc::util::position;
    static constexpr auto value = object(&t::side, &t::ticker, &t::quantity, &t::price);
};
