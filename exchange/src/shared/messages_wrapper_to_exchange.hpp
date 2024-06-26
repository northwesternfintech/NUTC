#pragma once

#include "ticker.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>

namespace nutc {
namespace messages {

/**
 * @brief Sent by clients to the exchange to indicate they're initialized
 * TODO: remove/replace
 */
struct init_message {
    bool placeholder = false;
    consteval init_message() = default;
};

struct market_order {
    util::Side side;
    util::Ticker ticker;
    double quantity;
    double price;

    bool operator==(const market_order& other) const = default;

    market_order(util::Side side, util::Ticker ticker, double quantity, double price) :
        side(side), ticker(ticker), quantity(quantity), price(price)
    {}

    market_order() = default;
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::market_order> {
    using t = nutc::messages::market_order;
    static constexpr auto value = object(&t::ticker, &t::side, &t::quantity, &t::price);
};

/// \cond
template <>
struct glz::meta<nutc::messages::init_message> {
    using t = nutc::messages::init_message;
    static constexpr auto value = object(&t::placeholder);
};
