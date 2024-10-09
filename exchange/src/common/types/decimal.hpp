#pragma once
#include <glaze/glaze.hpp>

#include <cmath>
#include <cstdint>

#include <functional>
#include <stdexcept>

#define PRICE_DECIMAL_PLACES    2
#define QUANTITY_DECIMAL_PLACES 2

namespace nutc::common {

namespace detail {
template <typename T>
consteval T
pow10(int pow)
{
    if (pow < 0) {
        throw std::invalid_argument("N must be non-negative");
    }
    return pow == 0 ? 1 : 10 * pow10<T>(pow - 1);
}
} // namespace detail

// THIS CLASS PRIORITIZES PRECISION OVER AVOIDING OVERFLOW
template <std::uint8_t Scale>
class Decimal {
    using decimal_type = std::int64_t;
    static constexpr std::int64_t MULTIPLIER = detail::pow10<decimal_type>(Scale);

    decimal_type value_{};

public:
    constexpr Decimal() = default;

    constexpr Decimal(double value) : value_(double_to_decimal(value)) {}

    Decimal operator-() const;
    decimal_type get_underlying() const;
    void set_underlying(decimal_type value);
    Decimal operator-(const Decimal& other) const;
    Decimal operator+(const Decimal& other) const;
    Decimal operator/(const Decimal& other) const;
    Decimal operator*(const Decimal& other) const;
    Decimal& operator*=(const Decimal& other);
    Decimal& operator/=(const Decimal& other);
    Decimal& operator+=(const Decimal& other);
    bool operator==(double other) const;

    explicit operator double() const;
    explicit operator float() const;
    auto operator<=>(const Decimal& other) const = default;
    bool operator==(const Decimal& other) const = default;

    Decimal difference(const Decimal& other) const;

private:
    constexpr explicit Decimal(decimal_type value) : value_(value) {}

    static constexpr bool
    double_within_bounds(double value)
    {
        bool lower_bound =
            (value * static_cast<double>(MULTIPLIER))
            > static_cast<double>(std::numeric_limits<decimal_type>::min());
        bool upper_bound =
            (value * static_cast<double>(MULTIPLIER))
            < static_cast<double>(std::numeric_limits<decimal_type>::max());

        return lower_bound && upper_bound;
    }

    static constexpr decimal_type
    double_to_decimal(double value)
    {
        if consteval {
            return static_cast<decimal_type>(value * MULTIPLIER);
        }

        if (!double_within_bounds(value)) [[unlikely]] {
            throw std::invalid_argument("value_ out of bounds");
        }

        return static_cast<decimal_type>(
            std::round(value * static_cast<double>(MULTIPLIER))
        );
    }

    friend std::hash<Decimal<Scale>>;
    friend std::numeric_limits<Decimal<Scale>>;
    friend glz::meta<nutc::common::Decimal<Scale>>;
};

using decimal_price = Decimal<PRICE_DECIMAL_PLACES>;
using decimal_quantity = Decimal<QUANTITY_DECIMAL_PLACES>;

} // namespace nutc::common

namespace std {
template <std::uint8_t Scale>
struct hash<nutc::common::Decimal<Scale>> {
    std::size_t
    operator()(const nutc::common::Decimal<Scale>& obj) const
    {
        return std::hash<int64_t>{}(obj.value_);
    }
};

// TODO: add unit tests
template <std::uint8_t Scale>
class numeric_limits<nutc::common::Decimal<Scale>> {
    using scaled_decimal = nutc::common::Decimal<Scale>;

public:
    static scaled_decimal
    max()
    {
        return scaled_decimal{
            std::numeric_limits<typename scaled_decimal::decimal_type>::max()
        };
    }

    static scaled_decimal
    min()
    {
        return scaled_decimal{
            std::numeric_limits<typename scaled_decimal::decimal_type>::min()
        };
    }
};

} // namespace std

/// \cond
template <std::uint8_t Scale>
struct glz::meta<nutc::common::Decimal<Scale>> {
    using t = nutc::common::Decimal<Scale>;
    static constexpr auto value = object(&t::value_);
};
