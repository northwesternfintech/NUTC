#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>

#include <functional>

// We round prices to 2 decimal places, so just store them as integers
struct decimal_price {
    uint32_t price{};

    constexpr decimal_price(double price) :
        price(static_cast<uint32_t>(std::round(price * 100)))
    {
        assert(price >= 0);
    }

    constexpr decimal_price() = default;

    operator double() const { return static_cast<double>(price) / 100; }
};

namespace std {
template <>
struct hash<decimal_price> {
    std::size_t
    operator()(const decimal_price& dp) const noexcept
    {
        return std::hash<uint32_t>()(dp.price);
    }
};
} // namespace std
