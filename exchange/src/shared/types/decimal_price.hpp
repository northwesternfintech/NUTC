#pragma once

#include <glaze/glaze.hpp>

#include <cassert>
#include <cmath>
#include <cstdint>

#include <functional>

namespace nutc {
namespace util {

// We round prices to 2 decimal places, so just store them as integers
struct decimal_price {
    static constexpr uint16_t MAX_ORDER_PRICE = std::numeric_limits<uint16_t>::max();

    uint32_t price{};

    // A bit hacky but necessary to support constexpr constructors
    constexpr uint32_t
    price_to_decimal(double price) const
    {
        assert(price >= 0);
        assert(price * 100 < std::numeric_limits<uint32_t>::max());
        if consteval {
            return static_cast<uint32_t>(price * 100);
        }
        else {
            return static_cast<uint32_t>(std::round(price * 100));
        }
    }

    constexpr decimal_price(double price_double) : price(price_to_decimal(price_double))
    {
        assert(price_double < std::numeric_limits<uint32_t>::max());
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

    bool
    valid_start_price() const
    {
        return price <= std::numeric_limits<uint16_t>::max();
    }

private:
    constexpr decimal_price(uint32_t decimal) : price(decimal) {}

    friend class std::numeric_limits<decimal_price>;
};
} // namespace util
} // namespace nutc

namespace std {
using decimal_price = nutc::util::decimal_price;

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
        return static_cast<uint32_t>(0);
    }

    static consteval decimal_price
    max() noexcept
    {
        return static_cast<uint32_t>(decimal_price::MAX_ORDER_PRICE);
    }
};
} // namespace std

/// \cond
template <>
struct glz::meta<nutc::util::decimal_price> {
    using t = nutc::util::decimal_price;
    static constexpr auto value = object(&t::price);
};
