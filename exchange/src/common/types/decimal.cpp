#include "decimal.hpp"

namespace nutc::common {

template <std::uint8_t Scale>
Decimal<Scale>
Decimal<Scale>::operator-() const
{
    return Decimal{-value_};
}

template <std::uint8_t Scale>
typename Decimal<Scale>::decimal_type
Decimal<Scale>::get_underlying() const
{
    return value_;
}

template <std::uint8_t Scale>
void
Decimal<Scale>::set_underlying(decimal_type value)
{
    value_ = value;
}

template <std::uint8_t Scale>
Decimal<Scale>
Decimal<Scale>::operator-(const Decimal<Scale>& other) const
{
    return Decimal{value_ - other.value_};
}

template <std::uint8_t Scale>
Decimal<Scale>
Decimal<Scale>::operator+(const Decimal<Scale>& other) const
{
    return Decimal{value_ + other.value_};
}

template <std::uint8_t Scale>
Decimal<Scale>
Decimal<Scale>::operator/(const Decimal<Scale>& other) const
{
    return Decimal{value_ * MULTIPLIER / other.value_};
}

template <std::uint8_t Scale>
Decimal<Scale>
Decimal<Scale>::operator*(const Decimal<Scale>& other) const
{
    return Decimal{(value_ * other.value_) / MULTIPLIER};
}

template <std::uint8_t Scale>
Decimal<Scale>&
Decimal<Scale>::operator*=(const Decimal<Scale>& other)
{
    value_ *= other.value_;
    value_ /= MULTIPLIER;
    return *this;
}

template <std::uint8_t Scale>
Decimal<Scale>&
Decimal<Scale>::operator/=(const Decimal<Scale>& other)
{
    value_ *= MULTIPLIER;
    value_ /= other.value_;
    return *this;
}

template <std::uint8_t Scale>
Decimal<Scale>&
Decimal<Scale>::operator+=(const Decimal<Scale>& other)
{
    value_ += other.value_;
    return *this;
}

template <std::uint8_t Scale>
bool
Decimal<Scale>::operator==(double other) const
{
    return value_ == static_cast<decimal_type>(other * MULTIPLIER);
}

template <std::uint8_t Scale>
Decimal<Scale>::operator double() const
{
    return static_cast<double>(value_) / static_cast<double>(MULTIPLIER);
}

template <std::uint8_t Scale>
Decimal<Scale>::operator float() const
{
    return static_cast<float>(value_) / static_cast<float>(MULTIPLIER);
}

template <std::uint8_t Scale>
Decimal<Scale>
Decimal<Scale>::difference(const Decimal<Scale>& other) const
{
    if (value_ >= other.value_) {
        return Decimal{value_ - other.value_};
    }
    return Decimal{other.value_ - value_};
}

template class Decimal<PRICE_DECIMAL_PLACES>;
template class Decimal<HIGH_PRECISION_DECIMAL>;

#if PRICE_DECIMAL_PLACES != QUANTITY_DECIMAL_PLACES
template class Decimal<QUANTITY_DECIMAL_PLACES>;
#endif

} // namespace nutc::common
