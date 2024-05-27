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
    ) :
        trader(std::move(trader)),
        ticker(std::move(ticker)), side(side), price(std::round(price * 100) / 100),
        quantity(quantity), tick(tick), order_index(get_and_increment_global_index())
    {}

    stored_order(const stored_order& other) = default;
    stored_order& operator=(const stored_order& other) = default;

    bool
    operator==(const stored_order& other) const
    {
        return trader->get_id() == other.trader->get_id() && ticker == other.ticker
               && side == other.side && util::is_close_to_zero(price - other.price)
               && util::is_close_to_zero(quantity - other.quantity);
    }

    int
    operator<=>(const stored_order& other) const
    {
        // assuming both sides are same
        // otherwise, this shouldn't even be called
        if (util::is_close_to_zero(this->price - other.price)) {
            if (this->order_index > other.order_index)
                return -1;
            if (this->order_index < other.order_index)
                return 1;
            return 0;
        }
        if (this->side == util::Side::buy) {
            if (this->price < other.price)
                return -1;
            if (this->price > other.price)
                return 1;
            return 0;
        }
        if (this->side == util::Side::sell) {
            if (this->price > other.price)
                return -1;
            if (this->price < other.price)
                return 1;
            return 0;
        }
        return 0;
    }

    [[nodiscard]] bool
    can_match(const stored_order& other) const
    {
        if (this->side == other.side) [[unlikely]] {
            return false;
        }
        if (this->ticker != other.ticker) [[unlikely]] {
            return false;
        }
        if (this->side == util::Side::buy && this->price < other.price) {
            return false;
        }
        if (this->side == util::Side::sell && this->price > other.price) {
            return false;
        }
        return true;
    }

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
