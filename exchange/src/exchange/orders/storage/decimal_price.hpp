#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>

#include <functional>

namespace nutc {
namespace matching {

// We round prices to 2 decimal places, so just store them as integers
struct decimal_price {
    uint32_t price{};

    // A bit hacky but necessary to support constexpr constructors
    constexpr uint32_t
    price_to_decimal(double price) const
    {
        if consteval {
            return static_cast<uint32_t>(price * 100);
        }
        else {
            return static_cast<uint32_t>(std::round(price * 100));
        }
    }

    constexpr decimal_price(double price) : price(price_to_decimal(price))
    {
        assert(price >= 0);
    }

    constexpr decimal_price() = default;

    decimal_price
    operator*(const decimal_price& other) const
    {
        return (price * other.price) / 100;
    }

    decimal_price
    operator-(const decimal_price& other) const
    {
        return price - other.price;
    }

    decimal_price
    operator+(const decimal_price& other) const
    {
        return price + other.price;
    }

    bool
    operator==(const decimal_price& other) const
    {
        return price == other.price;
    }

    bool
    operator==(double other) const
    {
        return price == static_cast<uint32_t>(other * 100);
    }

    operator double() const { return static_cast<double>(price) / 100; }

private:
    constexpr decimal_price(uint32_t decimal) : price(decimal) {}

    friend class std::numeric_limits<decimal_price>;
};
} // namespace matching
} // namespace nutc

namespace std {
using decimal_price = nutc::matching::decimal_price;

template <>
struct hash<decimal_price> {
    std::size_t
    operator()(const decimal_price& dp) const noexcept
    {
        return std::hash<uint32_t>()(dp.price);
    }
};

template <>
class numeric_limits<decimal_price> {
public:
    static consteval decimal_price
    min() noexcept
    {
        return std::numeric_limits<uint32_t>::min();
    }

    static consteval decimal_price
    max() noexcept
    {
        return std::numeric_limits<uint32_t>::max();
    }
};
} // namespace std
