#pragma once

#include "types/position.hpp"
#include "types/ticker.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>

namespace nutc {
namespace messages {

struct init_message {
    bool successful_startup;
};

struct market_order {
    util::Side side;
    util::Ticker ticker;
    double quantity;
};

struct limit_order {
    util::position position;
    bool ioc;

    bool operator==(const limit_order& other) const = default;

    limit_order(
        util::Side side, util::Ticker ticker, double quantity,
        util::decimal_price price, bool ioc = false
    ) : position{side, ticker, quantity, price}, ioc(ioc)
    {}

    limit_order(const util::position& position, bool ioc = false) :
        position(position), ioc(ioc)
    {}

    limit_order() = default;
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::limit_order> {
    using t = nutc::messages::limit_order;
    static constexpr auto value = object(&t::position, &t::ioc);
};

/// \cond
template <>
struct glz::meta<nutc::messages::market_order> {
    using t = nutc::messages::market_order;
    static constexpr auto value = object(&t::side, &t::ticker, &t::quantity);
};

/// \cond
template <>
struct glz::meta<nutc::messages::init_message> {
    using t = nutc::messages::init_message;
    static constexpr auto value = object(&t::successful_startup);
};
