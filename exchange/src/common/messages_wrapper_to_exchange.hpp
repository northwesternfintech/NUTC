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

struct cancel_order {
    Ticker ticker;
    order_id_t order_id;
    std::uint64_t timestamp;

    cancel_order(Ticker ticker, order_id_t order_id, std::uint64_t timestamp) :
        ticker(ticker), order_id(order_id), timestamp(timestamp)
    {}

    cancel_order() = default;

    bool
    operator==(const cancel_order& other) const
    {
        return ticker == other.ticker && order_id == other.order_id;
    }
};

struct market_order {
    Ticker ticker;
    Side side;
    decimal_quantity quantity;
    std::uint64_t timestamp;

    constexpr market_order() = default;

    market_order(
        Ticker ticker, Side side, decimal_quantity quantity, std::uint64_t timestamp
    ) : ticker(ticker), side(side), quantity(quantity), timestamp(timestamp)
    {}

    bool
    operator==(const market_order& other) const
    {
        return ticker == other.ticker && side == other.side
               && quantity == other.quantity;
    }
};

struct limit_order : market_order {
    decimal_price price;
    bool ioc{false};
    order_id_t order_id;

    // TODO: fix tests and remove
    bool
    operator==(const limit_order& other) const
    {
        return ticker == other.ticker && side == other.side
               && quantity == other.quantity && price == other.price
               && ioc == other.ioc;
    }

    limit_order(
        Ticker ticker, Side side, decimal_quantity quantity, decimal_price price,
        bool ioc, std::uint64_t timestamp, order_id_t order_id
    ) :
        market_order{ticker, side, quantity, timestamp}, price{price}, ioc{ioc},
        order_id{order_id}
    {}

    limit_order() = default;
};

using IncomingMessageVariant =
    std::variant<init_message, cancel_order, limit_order, market_order>;

} // namespace nutc::common

/// \cond
template <>
struct glz::meta<nutc::common::cancel_order> {
    using t = nutc::common::cancel_order;
    static constexpr auto value =
        object("cancel", &t::ticker, &t::order_id, &t::timestamp);
};

/// \cond
template <>
struct glz::meta<nutc::common::limit_order> {
    using t = nutc::common::limit_order;
    static constexpr auto value = object(
        "limit", &t::ticker, &t::side, &t::quantity, &t::timestamp, &t::price, &t::ioc,
        &t::order_id
    );
};

/// \cond
template <>
struct glz::meta<nutc::common::market_order> {
    using t = nutc::common::market_order;
    static constexpr auto value =
        object("market", &t::ticker, &t::side, &t::quantity, &t::timestamp);
};

/// \cond
template <>
struct glz::meta<nutc::common::init_message> {
    using t = nutc::common::init_message;
    static constexpr auto value = object(&t::name);
};
