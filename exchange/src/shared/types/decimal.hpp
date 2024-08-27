#pragma once
#include <glaze/glaze.hpp>

#include <cmath>
#include <cstdint>

#include <stdexcept>

namespace nutc::shared {
static constexpr auto PRICE_DECIMAL_PLACES = 2;
static constexpr auto QUANTITY_DECIMAL_PLACES = 2;

namespace {
template <typename T>
consteval T
pow10(int pow)
{
    if (pow < 0) {
        throw std::invalid_argument("N must be non-negative");
    }
    return pow == 0 ? 1 : 10 * pow10<T>(pow - 1);
}
} // namespace

template <std::int8_t Scale>
class Decimal {
    using decimal_type = std::int64_t;
    static constexpr std::int64_t MULTIPLIER = pow10<decimal_type>(Scale);

    decimal_type value_{};

public:
    constexpr Decimal() = default;

    constexpr Decimal(double value) : value_(double_to_decimal(value)) {}

    Decimal
    operator-() const
    {
        return -value_;
    }

    decimal_type
    get_underlying() const
    {
        return value_;
    }

    void
    set_underlying(decimal_type value)
    {
        value_ = value;
    }

    constexpr Decimal
    operator-(const Decimal& other) const
    {
        return value_ - other.value_;
    }

    constexpr Decimal
    operator+(const Decimal& other) const
    {
        return value_ + other.value_;
    }

    constexpr Decimal
    operator/(const Decimal& other) const
    {
        return value_ / other.value_;
    }

    constexpr Decimal
    operator*(const Decimal& other) const
    {
        return (value_ * other.value_) / MULTIPLIER;
    }

    constexpr Decimal&
    operator/=(const Decimal& other)
    {
        value_ /= other.value_;
        return *this;
    }

    constexpr Decimal&
    operator+=(const Decimal& other)
    {
        value_ += other.value_;
        return *this;
    }

    constexpr bool
    operator==(double other) const
    {
        return value_ == static_cast<decimal_type>(other * MULTIPLIER);
    }

    explicit constexpr
    operator double() const
    {
        return static_cast<double>(value_) / static_cast<double>(MULTIPLIER);
    }

    explicit constexpr
    operator float() const
    {
        return static_cast<float>(value_) / static_cast<float>(MULTIPLIER);
    }

    constexpr auto operator<=>(const Decimal& other) const = default;
    constexpr bool operator==(const Decimal& other) const = default;

    Decimal
    difference(const Decimal& other) const
    {
        if (value_ >= other.value_) {
            return value_ - other.value_;
        }
        return other.value_ - value_;
    }

private:
    constexpr Decimal(decimal_type value) : value_(value) {}

    static bool
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
        // Necessary to support constexpr constructors
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
    friend glz::meta<nutc::shared::Decimal<Scale>>;
};

using decimal_price = Decimal<PRICE_DECIMAL_PLACES>;
using decimal_quantity = Decimal<QUANTITY_DECIMAL_PLACES>;
} // namespace nutc::shared

namespace std {
template <std::int8_t Scale>
struct hash<nutc::shared::Decimal<Scale>> {
    std::size_t
    operator()(const nutc::shared::Decimal<Scale>& obj) const
    {
        return std::hash<int64_t>{}(obj.value_);
    }
};
} // namespace std

/// \cond
template <std::int8_t Scale>
struct glz::meta<nutc::shared::Decimal<Scale>> {
    using t = nutc::shared::Decimal<Scale>;
    static constexpr auto value = object(&t::value_);
};
