#include "ticker.hpp"

namespace nutc {
namespace util {

bool
Ticker::operator==(const Ticker& other) const
{
    return arr == other.arr;
}

Ticker::Ticker(const char* str)
{
    for (size_t i = 0; i < TICKER_LENGTH; i++) {
        arr[i] = str[i];
    }
}

Ticker::Ticker(std::string_view str)
{
    for (size_t i = 0; i < TICKER_LENGTH; i++) {
        arr[i] = str[i];
    }
}

Ticker::operator std::string() const
{
    std::string str(TICKER_LENGTH, '0');
    for (size_t i = 0; i < TICKER_LENGTH; i++) {
        str[i] = arr[i];
    }
    return str;
}

} // namespace util
} // namespace nutc
