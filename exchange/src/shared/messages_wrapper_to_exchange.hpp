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

struct limit_order {
    util::Side side;
    util::Ticker ticker;
    double price;
    double quantity;

    bool operator==(const limit_order& other) const = default;

    limit_order(util::Side side, util::Ticker ticker, double price, double quantity) :
        side(side), ticker(ticker), price(price), quantity(quantity)
    {}

    limit_order() = default;
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::limit_order> {
    using t = nutc::messages::limit_order;
    static constexpr auto value = object(&t::ticker, &t::side, &t::price, &t::quantity);
};

/// \cond
template <>
struct glz::meta<nutc::messages::init_message> {
    using t = nutc::messages::init_message;
    static constexpr auto value = object(&t::placeholder);
};
