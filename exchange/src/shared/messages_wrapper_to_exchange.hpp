#pragma once

#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>

namespace nutc {
namespace messages {

/**
 * @brief Sent by clients to the exchange to indicate they're initialized and may or may
 * not be participating in the competition
 */
struct InitMessage {
    std::string client_id;
    bool ready;
};

/**
 * @brief Sent by clients to the exchange to place an order
 * TODO: client_id=="SIMULATED" indicates simulated order with no actual
 * owner, but this is improper. Instead, it should be an optional
 */
struct MarketOrder {
    std::string client_id;
    std::string ticker;
    SIDE side;
    float price;
    float quantity;

    // Used to sort orders by time created
    long long order_index;

    MarketOrder() { order_index = get_and_increment_global_index(); }

    static long long
    get_and_increment_global_index()
    {
        static long long global_index = 0;
        return global_index++;
    }

    MarketOrder(
        const std::string& client_id, SIDE side, const std::string& ticker,
        float quantity, float price
    ) :
        client_id(client_id),
        ticker(ticker), side(side), price(price), quantity(quantity)
    {
        order_index = get_and_increment_global_index();
    }

    bool
    operator==(const MarketOrder& other) const
    {
        return client_id == other.client_id && ticker == other.ticker
               && side == other.side && util::is_close_to_zero(price - other.price)
               && util::is_close_to_zero(quantity - other.quantity);
    }

    // toString
    std::string
    to_string() const
    {
        std::string side_str = side == SIDE::BUY ? "BUY" : "SELL";
        return fmt::format(
            "MarketOrder(client_id={}, side={}, ticker={}, quantity={}, "
            "price={})",
            client_id, side_str, ticker, quantity, price
        );
    }

    bool
    operator<(const MarketOrder& other) const
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

    bool
    can_match(const MarketOrder& other) const
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

    // To ensure we don't increment the client_id
    MarketOrder(const MarketOrder& other) : order_index(other.order_index)
    {
        this->client_id = other.client_id;
        this->side = other.side;
        this->ticker = other.ticker;
        this->quantity = other.quantity;
        this->price = other.price;
    }

    MarketOrder&
    operator=(const MarketOrder& other)
    {
        if (this == &other) {
            return *this;
        }

        this->order_index = other.order_index;
        this->client_id = other.client_id;
        this->side = other.side;
        this->ticker = other.ticker;
        this->quantity = other.quantity;
        this->price = other.price;

        return *this;
    }
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::MarketOrder> {
    using T = nutc::messages::MarketOrder;
    static constexpr auto value = object(
        "client_id", &T::client_id, "side", &T::side, "ticker", &T::ticker, "quantity",
        &T::quantity, "price", &T::price
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::InitMessage> {
    using T = nutc::messages::InitMessage;
    static constexpr auto value =
        object("client_id", &T::client_id, "ready", &T::ready);
};
