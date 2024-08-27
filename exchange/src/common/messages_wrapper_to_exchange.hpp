#pragma once

#include "common/util.hpp"
#include "types/decimal.hpp"
#include "types/ticker.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>
#include <glaze/util/type_traits.hpp>

namespace nutc::common {

struct init_message {
    std::string_view name = "init_message";
};

struct market_order {
    common::Ticker ticker;
    common::Side side;
    decimal_quantity quantity;
    std::uint64_t timestamp;

    constexpr market_order() = default;

    market_order(common::Ticker ticker, common::Side side, double quantity) :
        ticker(ticker), side(side), quantity(quantity), timestamp(get_time())
    {}

    bool
    operator==(const market_order& other) const
    {
        return ticker == other.ticker && side == other.side
               && quantity == other.quantity;
    }
};

struct limit_order : market_order {
    common::decimal_price price;
    bool ioc{false};

    bool operator==(const limit_order& other) const = default;

    limit_order(
        std::string_view ticker, common::Side side, double quantity,
        common::decimal_price price, bool ioc = false
    ) :
        market_order{common::force_to_ticker(ticker), side, quantity}, price{price},
        ioc{ioc}
    {}

    limit_order(
        common::Ticker ticker, common::Side side, double quantity,
        common::decimal_price price, bool ioc = false
    ) : market_order{ticker, side, quantity}, price{price}, ioc{ioc}
    {}

    limit_order() = default;
};

using IncomingMessageVariant = std::variant<init_message, limit_order, market_order>;

} // namespace nutc::common

/// \cond
template <>
struct glz::meta<nutc::common::limit_order> {
    using t = nutc::common::limit_order;
    static constexpr auto value = object(
        "limit", &t::timestamp, &t::ticker, &t::side, &t::quantity, &t::price, &t::ioc
    );
};

/// \cond
template <>
struct glz::meta<nutc::common::market_order> {
    using t = nutc::common::market_order;
    static constexpr auto value =
        object("market", &t::timestamp, &t::ticker, &t::side, &t::quantity);
};

/// \cond
template <>
struct glz::meta<nutc::common::init_message> {
    using t = nutc::common::init_message;
    static constexpr auto value = object(&t::name);
};
