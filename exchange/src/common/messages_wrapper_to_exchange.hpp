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
    std::uint64_t timestamp = get_time();

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
    std::uint64_t timestamp = get_time();

    constexpr market_order() = default;

    market_order(Ticker ticker, Side side, decimal_quantity quantity) :
        ticker(ticker), side(side), quantity(quantity)
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
    order_id_t order_id = generate_order_id();

    // TODO: fix tests and remove
    bool
    operator==(const limit_order& other) const
    {
        return ticker == other.ticker && side == other.side
               && quantity == other.quantity && price == other.price
               && ioc == other.ioc;
    }

    limit_order(
        std::string_view ticker, Side side, decimal_quantity quantity,
        decimal_price price, bool ioc = false
    ) : market_order{force_to_ticker(ticker), side, quantity}, price{price}, ioc{ioc}
    {}

    limit_order(
        Ticker ticker, Side side, decimal_quantity quantity, decimal_price price,
        bool ioc = false
    ) : market_order{ticker, side, quantity}, price{price}, ioc{ioc}
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
    static constexpr auto value = object("cancel", &t::ticker, &t::order_id);
};

/// \cond
template <>
struct glz::meta<nutc::common::limit_order> {
    using t = nutc::common::limit_order;
    static constexpr auto value = object(
        "limit", &t::timestamp, &t::ticker, &t::side, &t::quantity, &t::price, &t::ioc,
        &t::order_id
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
