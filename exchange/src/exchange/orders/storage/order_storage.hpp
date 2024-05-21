#pragma once
#include "exchange/orders/storage/decimal_price.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <fmt/format.h>

namespace nutc {
namespace matching {

struct stored_match {
    traders::GenericTrader& buyer;
    traders::GenericTrader& seller;
    util::Ticker ticker;
    util::Side side;
    decimal_price price;
    double quantity;
};

struct stored_order {
    traders::GenericTrader& trader;
    util::Ticker ticker;
    util::Side side;
    decimal_price price;
    double quantity;
    uint64_t tick;
    bool active{true};

    // Used to sort orders by time created
    uint64_t order_index;

    operator messages::limit_order() const { return {side, ticker, price, quantity}; }

    static uint64_t
    get_and_increment_global_index()
    {
        static uint64_t global_index = 0;
        return global_index++;
    }

    stored_order(
        traders::GenericTrader& trader, util::Ticker ticker, util::Side side,
        double price, double quantity, uint64_t tick = 0
    );

    stored_order(const stored_order& other) = default;

    bool operator==(const stored_order& other) const;

    int operator<=>(const stored_order& other) const;

    [[nodiscard]] bool can_match(const stored_order& other) const;

    ~stored_order() = default;
};

struct order_index {
    decimal_price price;
    uint64_t index;
};

// Want highest first
struct bid_comparator {
    bool
    operator()(const order_index& lhs, const order_index& rhs) const
    {
        if (lhs.price != rhs.price) {
            return lhs.price > rhs.price;
        }
        return lhs.index < rhs.index;
    }
};

// Want lowest first
struct ask_comparator {
    bool
    operator()(const order_index& lhs, const order_index& rhs) const
    {
        if (lhs.price != rhs.price) {
            return lhs.price < rhs.price;
        }
        return lhs.index < rhs.index;
    }
};
} // namespace matching
} // namespace nutc
