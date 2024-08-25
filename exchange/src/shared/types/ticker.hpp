#pragma once

#include "shared/config/config.h"

#include <glaze/glaze.hpp>

#include <cassert>

#include <array>
#include <string_view>

namespace nutc::shared {
struct Ticker {
    std::array<char, TICKER_LENGTH> arr{};

    constexpr Ticker() = default;

    constexpr Ticker(std::array<char, TICKER_LENGTH> arr) : arr(arr) {}

    Ticker(std::string_view str);
    Ticker(const char* str);

    operator std::array<char, TICKER_LENGTH>() const { return arr; }

    bool operator==(const Ticker& other) const;
    operator std::string() const;
};
} // namespace nutc::shared

/// \cond
template <>
struct glz::meta<nutc::shared::Ticker> {
    using t = nutc::shared::Ticker;
    static constexpr auto value = object(&t::arr);
};

// Simple because we do a lot of hashing but have very few tickers. Maybe revisit later?
namespace std {
template <>
struct hash<nutc::shared::Ticker> {
    std::size_t
    operator()(const nutc::shared::Ticker arr) const noexcept
    {
        std::size_t hash_value = 0;
        for (size_t i = 0; i < TICKER_LENGTH; i++) {
            hash_value += static_cast<uint8_t>(arr.arr[i]);
        }
        return hash_value;
    }
};
} // namespace std
