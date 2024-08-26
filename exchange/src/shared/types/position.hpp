#pragma once

#include "shared/types/decimal.hpp"
#include "shared/types/ticker.hpp"
#include "shared/util.hpp"

#include <glaze/glaze.hpp>

namespace nutc::shared {
struct position {
    Ticker ticker;
    Side side;
    double quantity;
    shared::decimal_price price;

    bool
    operator==(const position& other) const noexcept
    {
        return side == other.side && ticker == other.ticker && price == other.price
               && ((quantity - other.quantity) == 0);
    }

    position() = default;

    position(Ticker ticker, Side side, double quantity, shared::decimal_price price) :
        ticker(ticker), side(side), quantity(quantity), price(price)
    {}
};

} // namespace nutc::shared

/// \cond
template <>
struct glz::meta<nutc::shared::position> {
    using t = nutc::shared::position;
    static constexpr auto value = object(&t::side, &t::ticker, &t::quantity, &t::price);
};
