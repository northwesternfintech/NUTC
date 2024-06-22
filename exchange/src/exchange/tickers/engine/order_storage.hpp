#pragma once
#include "exchange/traders/trader_types/generic_trader.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <fmt/format.h>

#include <string>

namespace nutc {
namespace matching {

struct stored_match {
    std::shared_ptr<traders::GenericTrader> buyer;
    std::shared_ptr<traders::GenericTrader> seller;
    std::string ticker;
    util::Side side;
    double price;
    double quantity;
};

struct stored_order {
    std::shared_ptr<traders::GenericTrader> trader;
    std::string ticker{};
    util::Side side{};
    double price{};
    double quantity{};
    uint64_t tick{};

    // Used to sort orders by time created
    uint64_t order_index{};

    stored_order() = default;

    operator messages::market_order() const { return {side, ticker, quantity, price}; }

    static uint64_t
    get_and_increment_global_index()
    {
        static uint64_t global_index = 0;
        return global_index++;
    }

    stored_order(
        std::shared_ptr<traders::GenericTrader> trader, util::Side side,
        std::string ticker, double quantity, double price, uint64_t tick = 0
    );

    stored_order(const stored_order& other) = default;
    stored_order& operator=(const stored_order& other) = default;

    bool operator==(const stored_order& other) const;

    int operator<=>(const stored_order& other) const;

    [[nodiscard]] bool can_match(const stored_order& other) const;

    ~stored_order() = default;
};

struct order_index {
    double price;
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
