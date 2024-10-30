#pragma once

#include "common/types/decimal.hpp"
#include "common/types/ticker.hpp"
#include "common/util.hpp"

#include <fmt/format.h>

namespace nutc::common {

struct init_message {
    std::string_view name = "init_message";
};

struct cancel_order {
    Ticker ticker;
    order_id_t order_id;
    std::uint64_t timestamp;

    cancel_order() = default;

    cancel_order(Ticker ticker, order_id_t order_id) :
        ticker(ticker), order_id(order_id), timestamp(get_time())
    {}

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

    market_order() = default;

    market_order(Ticker ticker, Side side, decimal_quantity quantity) :
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
    decimal_price price;
    bool ioc;
    order_id_t order_id;

    limit_order() = default;

    limit_order(
        Ticker ticker, Side side, decimal_quantity quantity, decimal_price price,
        bool ioc = false
    ) :
        market_order{ticker, side, quantity}, price{price}, ioc{ioc},
        order_id{generate_order_id()}
    {}

    // TODO: fix tests and remove
    bool
    operator==(const limit_order& other) const
    {
        return ticker == other.ticker && side == other.side
               && quantity == other.quantity && price == other.price
               && ioc == other.ioc;
    }
};

using IncomingMessageVariant =
    std::variant<init_message, cancel_order, limit_order, market_order>;

} // namespace nutc::common
