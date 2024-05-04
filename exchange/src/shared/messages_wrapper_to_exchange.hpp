#pragma once

#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>

namespace nutc {
namespace messages {

/**
 * @brief Sent by clients to the exchange to indicate they're initialized and may or may
 * not be participating in the competition
 */
struct init_message {
    int test{5};
    init_message() = default;
};

struct market_order {
    util::Side side;
    std::string ticker;
    double quantity;
    double price;

    market_order(util::Side side, std::string ticker, double quantity, double price) :
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
    static constexpr auto value = object("init", &t::test);
};
