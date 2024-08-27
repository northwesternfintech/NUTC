#pragma once

#include "common/types/decimal.hpp"
#include "common/types/ticker.hpp"
#include "common/util.hpp"

#include <glaze/glaze.hpp>

namespace nutc::common {
struct position {
    Ticker ticker;
    Side side;
    common::decimal_quantity quantity;
    common::decimal_price price;

    bool
    operator==(const position& other) const noexcept
    {
        return side == other.side && ticker == other.ticker && price == other.price
               && ((quantity - other.quantity) == 0.0);
    }

    position() = default;

    position(
        Ticker ticker, Side side, common::decimal_quantity quantity,
        common::decimal_price price
    ) : ticker(ticker), side(side), quantity(quantity), price(price)
    {}
};

} // namespace nutc::common

/// \cond
template <>
struct glz::meta<nutc::common::position> {
    using t = nutc::common::position;
    static constexpr auto value = object(&t::side, &t::ticker, &t::quantity, &t::price);
};
