#pragma once
#include "exchange/traders/trader_types/generic_trader.hpp"

#include <fmt/format.h>

#include <string>

namespace nutc {
namespace matching {

using SIDE = messages::SIDE;

struct StoredMatch {
    std::shared_ptr<manager::GenericTrader> buyer;
    std::shared_ptr<manager::GenericTrader> seller;
    std::string ticker;
    SIDE side;
    double price;
    double quantity;
};

struct StoredOrder {
    std::shared_ptr<manager::GenericTrader> trader;
    std::string ticker;
    SIDE side;
    double price;
    double quantity;
    uint64_t tick;

    // Used to sort orders by time created
    uint64_t order_index;

    StoredOrder() = default;

    static uint64_t
    get_and_increment_global_index()
    {
        static uint64_t global_index = 0;
        return global_index++;
    }

    StoredOrder(
        std::shared_ptr<manager::GenericTrader> trader, SIDE side, std::string ticker,
        double quantity, double price, uint64_t tick
    ) :
        trader(std::move(trader)),
        ticker(std::move(ticker)), side(side), price(price), quantity(quantity),
        tick(tick), order_index(get_and_increment_global_index())
    {}

    StoredOrder(StoredOrder&& other) noexcept :
        trader(std::move(other.trader)), ticker(std::move(other.ticker)),
        side(other.side), price(other.price), quantity(other.quantity),
        tick(other.tick), order_index(other.order_index)
    {}

    StoredOrder&
    operator=(StoredOrder&& other) noexcept
    {
        if (this != &other) {
            trader = std::move(other.trader);
            ticker = std::move(other.ticker);
            side = other.side;
            price = other.price;
            quantity = other.quantity;
            tick = other.tick;
            order_index = other.order_index;
        }
        return *this;
    }

    StoredOrder(const StoredOrder& other) = default;
    StoredOrder& operator=(const StoredOrder& other) = delete;

    bool
    operator==(const StoredOrder& other) const
    {
        return trader->get_id() == other.trader->get_id() && ticker == other.ticker
               && side == other.side && util::is_close_to_zero(price - other.price)
               && util::is_close_to_zero(quantity - other.quantity);
    }

    // toString
    [[nodiscard]] std::string
    to_string() const
    {
        std::string side_str = side == SIDE::BUY ? "BUY" : "SELL";
        return fmt::format(
            "StoredOrder(client_id={}, side={}, ticker={}, quantity={}, "
            "price={})",
            trader->get_id(), side_str, ticker, quantity, price
        );
    }

    int
    operator<=>(const StoredOrder& other) const
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
        if (this->side == SIDE::BUY) {
            if (this->price < other.price)
                return -1;
            if (this->price > other.price)
                return 1;
            return 0;
        }
        if (this->side == SIDE::SELL) {
            if (this->price > other.price)
                return -1;
            if (this->price < other.price)
                return 1;
            return 0;
        }
        return 0;
    }

    [[nodiscard]] bool
    can_match(const StoredOrder& other) const
    {
        if (this->side == other.side) [[unlikely]] {
            return false;
        }
        if (this->ticker != other.ticker) [[unlikely]] {
            return false;
        }
        if (this->side == SIDE::BUY && this->price < other.price) {
            return false;
        }
        if (this->side == SIDE::SELL && this->price > other.price) {
            return false;
        }
        return true;
    }
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
