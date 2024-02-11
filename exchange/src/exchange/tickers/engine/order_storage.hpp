#pragma once
#include "exchange/traders/trader_manager.hpp"
#include "exchange/traders/trader_types.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <fmt/format.h>

#include <string>

namespace nutc {
namespace matching {

using SIDE = messages::SIDE;

struct StoredOrder {
    const manager::generic_trader_t* trader;
    std::string ticker;
    SIDE side;
    float price;
    float quantity;
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
        const manager::generic_trader_t* trader, SIDE side, std::string ticker,
        float quantity, float price, uint64_t tick
    ) :
        trader(trader),
        ticker(std::move(ticker)), side(side), price(price), quantity(quantity),
        tick(tick), order_index(get_and_increment_global_index())
    {}

    explicit StoredOrder(messages::MarketOrder&& other, uint64_t tick) :

        trader(manager::ClientManager::get_instance().get_generic_trader(other.client_id
        )),
        ticker(std::move(other.ticker)), side(other.side), price(other.price),
        quantity(other.quantity), tick(tick),
        order_index(get_and_increment_global_index())
    {}

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

    StoredOrder(const StoredOrder& other) = default;
};

struct order_index {
    float price;
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