#pragma once

#include <cstdint>

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace nutc::common {
// NOTE: this must be the same as Side in template.hpp
enum class Ticker : std::uint8_t { ETH = 0, BTC = 1, LTC = 2 }; // NOLINT
static constexpr auto TICKERS = {Ticker::ETH, Ticker::BTC, Ticker::LTC};

inline std::string
to_string(Ticker ticker)
{
    switch (ticker) {
        case Ticker::ETH:
            return "ETH";
        case Ticker::BTC:
            return "BTC";
        case Ticker::LTC:
            return "LTC";
    }
    std::unreachable();
}

constexpr inline std::optional<Ticker>
to_ticker(std::string_view ticker_str)
{
    if (ticker_str == "ETH")
        return Ticker::ETH;
    if (ticker_str == "BTC")
        return Ticker::BTC;
    if (ticker_str == "LTC")
        return Ticker::LTC;

    return std::nullopt;
}

constexpr inline Ticker
force_to_ticker(std::string_view ticker_str)
{
    std::optional<Ticker> ticker = to_ticker(ticker_str);
    if (!ticker)
        throw std::invalid_argument("Invalid ticker string");
    return *ticker;
}
} // namespace nutc::common
