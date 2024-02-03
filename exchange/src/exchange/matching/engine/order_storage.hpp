#pragma once
#include "shared/messages_wrapper_to_exchange.hpp"

#include <fmt/format.h>

#include <string>

namespace nutc {
namespace matching {

using SIDE = messages::SIDE;

/**
 * @brief Sent by clients to the exchange to place an order
 * TODO: client_id=="SIMULATED" indicates simulated order with no actual
 * owner, but this is improper. Instead, it should be an optional
 */
struct StoredOrder {
    std::string client_id;
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
        std::string client_id, SIDE side, std::string ticker, float quantity,
        float price, uint64_t tick
    ) :
        client_id(std::move(client_id)),
        ticker(std::move(ticker)), side(side), price(price), quantity(quantity),
        tick(tick), order_index(get_and_increment_global_index())
    {}

    explicit StoredOrder(messages::MarketOrder&& other, uint64_t tick) :

        client_id(std::move(other.client_id)), ticker(std::move(other.ticker)),
        side(other.side), price(other.price), quantity(other.quantity), tick(tick),
        order_index(get_and_increment_global_index())
    {}

    bool
    operator==(const StoredOrder& other) const
    {
        return client_id == other.client_id && ticker == other.ticker
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
            client_id, side_str, ticker, quantity, price
        );
    }

    bool
    operator<(const StoredOrder& other) const
    {
        // assuming both sides are same
        // otherwise, this shouldn't even be called
        if (util::is_close_to_zero(this->price - other.price)) {
            return this->order_index > other.order_index;
        }
        else if (this->side == SIDE::BUY) {
            return this->price < other.price;
        }
        else if (this->side == SIDE::SELL) {
            return this->price > other.price;
        }
        else {
            return false;
        }
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

    StoredOrder&
    operator=(const StoredOrder& other)
    {
        if (this == &other) {
            return *this;
        }

        this->order_index = other.order_index;
        this->tick = other.tick;
        this->client_id = other.client_id;
        this->side = other.side;
        this->ticker = other.ticker;
        this->quantity = other.quantity;
        this->price = other.price;

        return *this;
    }
};

// Want highest first
struct bid_comparator {
    bool
    operator()(
        const std::pair<float, uint64_t>& lhs, const std::pair<float, uint64_t>& rhs
    ) const
    {
        if (lhs.first != rhs.first) {
            return lhs.first > rhs.first;
        }
        return lhs.second < rhs.second;
    }
};

// Want lowest first
struct ask_comparator {
    bool
    operator()(
        const std::pair<float, uint64_t>& lhs, const std::pair<float, uint64_t>& rhs
    ) const
    {
        if (lhs.first != rhs.first) {
            return lhs.first < rhs.first;
        }
        return lhs.second < rhs.second;
    }
};
} // namespace matching
} // namespace nutc
