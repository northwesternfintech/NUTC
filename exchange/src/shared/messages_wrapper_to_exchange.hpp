#pragma once

#include "types/position.hpp"
#include "types/ticker.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>
#include <glaze/util/type_traits.hpp>

namespace nutc {
namespace messages {

struct init_message {
    std::string_view name = "init_message";
};

struct market_order {
    util::Ticker ticker;
    util::Side side;
    double quantity;

    market_order() = default;

    market_order(util::Ticker ticker, util::Side side, double quantity) :
        ticker(ticker), side(side), quantity(quantity)
    {}
};

struct limit_order {
    util::position position;
    bool ioc;

    bool operator==(const limit_order& other) const = default;

    limit_order(
        util::Ticker ticker, util::Side side, double quantity,
        util::decimal_price price, bool ioc = false
    ) : position{ticker, side, quantity, price}, ioc(ioc)
    {}

    limit_order() = default;
};

template <typename OrderT>
struct timed_message : public OrderT {
    std::chrono::steady_clock::time_point time_received =
        std::chrono::steady_clock::now();

    template <typename... Args>
    timed_message(Args&&... args) : OrderT(std::forward<Args>(args)...)
    {}
};

using timed_init_message = timed_message<init_message>;
using timed_limit_order = timed_message<limit_order>;
using timed_market_order = timed_message<market_order>;

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
    static constexpr auto value = object(&t::name);
};
