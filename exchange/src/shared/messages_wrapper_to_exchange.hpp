#pragma once

#include "types/position.hpp"
#include "types/ticker.hpp"
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
    util::position position;
    bool ioc;

    bool operator==(const limit_order& other) const = default;

    limit_order(
        util::Side side, util::Ticker ticker, util::decimal_price price,
        double quantity, bool ioc = false
    ) : position{side, ticker, price, quantity}, ioc(ioc)
    {}

    limit_order(const util::position& position, bool ioc = false) :
        position(position), ioc(ioc)
    {}

    limit_order() = default;
};

inline limit_order
make_market_order(util::Side side, util::Ticker ticker, double quantity)
{
    util::decimal_price price = (side == util::Side::buy)
                                    ? std::numeric_limits<util::decimal_price>::max()
                                    : std::numeric_limits<util::decimal_price>::min();
    return limit_order{side, ticker, price, quantity, true};
}

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
struct glz::meta<nutc::messages::init_message> {
    using t = nutc::messages::init_message;
    static constexpr auto value = object(&t::placeholder);
};
