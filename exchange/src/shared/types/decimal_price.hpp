#pragma once

#include <glaze/glaze.hpp>

#include <cassert>
#include <cmath>
#include <cstdint>

#include <functional>

namespace nutc::shared {

// We round prices to 2 decimal places, so just store them as integers
struct decimal_price {
    using decimal_type = int64_t;

    decimal_type price;

    static bool
    price_within_bounds(double price)
    {
        bool lower_bound =
            (price * 100)
            > static_cast<double>(std::numeric_limits<decimal_type>::min());
        bool upper_bound =
            (price * 100)
            < static_cast<double>(std::numeric_limits<decimal_type>::max());

        return lower_bound && upper_bound;
    }

    static constexpr decimal_type
    price_to_decimal(double price)
    {
        // Necessary to support constexpr constructors
        if consteval {
            return static_cast<decimal_type>(price * 100);
        }

        if (!price_within_bounds(price)) [[unlikely]] {
            throw std::invalid_argument("Price out of bounds");
        }

        return static_cast<decimal_type>(std::round(price * 100));
    }

    constexpr decimal_price(double price_double) : price(price_to_decimal(price_double))
    {}

    constexpr decimal_price() = default;
    constexpr decimal_price(const decimal_price& other) = default;
    constexpr decimal_price(decimal_price&& other) = default;
    constexpr decimal_price& operator=(const decimal_price& other) = default;
    constexpr decimal_price& operator=(decimal_price&& other) = default;

    constexpr decimal_price
    operator-(const decimal_price& other) const
    {
        return price - other.price;
    }

    constexpr decimal_price
    operator+(const decimal_price& other) const
    {
        return price + other.price;
    }

    constexpr decimal_price
    operator/(const decimal_price& other) const
    {
        return price / other.price;
    }

    constexpr decimal_price
    operator*(const decimal_price& other) const
    {
        return (price * other.price) / 100;
    }

    constexpr decimal_price&
    operator/=(const decimal_price& other)
    {
        price /= other.price;
        return *this;
    }

    constexpr decimal_price&
    operator+=(const decimal_price& other)
    {
        price += other.price;
        return *this;
    }

    constexpr auto operator<=>(const decimal_price& other) const = default;
    constexpr bool operator==(const decimal_price& other) const = default;

    constexpr bool
    operator==(double other) const
    {
        return price == static_cast<decimal_type>(other * 100);
    }

    explicit constexpr
    operator double() const
    {
        return static_cast<double>(price) / 100;
    }

    explicit constexpr
    operator float() const
    {
        return static_cast<float>(price) / 100;
    }

    constexpr ~decimal_price() = default;

    // TODO: bad practice
    decimal_price
    difference(const decimal_price& other) const noexcept
    {
        if (price >= other.price) {
            return price - other.price;
        }
        else {
            return other.price - price;
        }
    }

private:
    constexpr decimal_price(decimal_type decimal) : price(decimal) {}
};
} // namespace nutc::shared

namespace std {
using decimal_price = nutc::shared::decimal_price;

template <>
struct hash<decimal_price> {
    std::size_t
    operator()(const decimal_price& dp) const noexcept
    {
        return std::hash<int64_t>()(dp.price);
    }
};

} // namespace std

/// \cond
template <>
struct glz::meta<nutc::shared::decimal_price> {
    using t = nutc::shared::decimal_price;
    static constexpr auto value = object(&t::price);
};
