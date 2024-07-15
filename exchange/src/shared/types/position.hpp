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
    util::decimal_price price;
    double quantity;

    bool
    operator==(const position& other) const noexcept
    {
        return side == other.side && ticker == other.ticker && price == other.price
               && ((quantity - other.quantity)==0);
    }

    position() = default;

    position(Side side, Ticker ticker, util::decimal_price price, double quantity) :
        side(side), ticker(ticker), price(price), quantity(quantity)
    {}
};


} // namespace util
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::util::position> {
    using t = nutc::util::position;
    static constexpr auto value = object(&t::side, &t::ticker, &t::price, &t::quantity);
};
