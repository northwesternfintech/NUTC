#pragma once

#include <fmt/format.h>
#include <glaze/glaze.hpp>

#include <iostream>

namespace nutc {

/**
 * @brief Contains all types used by glaze and the exchange for orders, matching,
 * communication, etc
 */
namespace messages {

enum class SIDE { BUY, SELL };

/**
 * @brief Sent by clients to the exchange to indicate they're initialized and may or may
 * not be participating in the competition
 */
struct InitMessage {
    std::string client_id;
    bool ready;
};

struct StartTime {
    long long start_time_ns;
};

/**
 * @brief Sent by exchange to a client to indicate a match has occurred
 */
struct Match {
    std::string ticker;
    SIDE side;
    float price;
    float quantity;
    std::string buyer_id;
    std::string seller_id;
};

inline constexpr bool
is_close_to_zero(float value, float epsilon = 1e-6f)
{
    return std::fabs(value) < epsilon;
}

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
        const std::string& client_id,
        SIDE side,
        const std::string& ticker,
        float quantity,
        float price
    ) :
        client_id(client_id),
        ticker(ticker),
        side(side),
        price(price),
        quantity(quantity)
    {
        order_index = get_and_increment_global_index();
    }

    // toString
    std::string
    to_string() const
    {
        std::string side_str = side == SIDE::BUY ? "BUY" : "SELL";
        return fmt::format(
            "MarketOrder(client_id={}, side={}, ticker={}, quantity={}, "
            "price={})",
            client_id,
            side_str,
            ticker,
            quantity,
            price
        );
    }

    bool
    operator<(const MarketOrder& other) const
    {
        // assuming both sides are same
        // otherwise, this shouldn't even be called
        if (is_close_to_zero(this->price - other.price)) {
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

/**
 * @brief Sent by exchange to clients to indicate an orderbook update
 */
struct ObUpdate {
    std::string ticker;
    SIDE side;
    float price;
    float quantity;
};

/**
 * @brief Sent by exchange to clients to indicate an update with their specific account
 * This is only sent to the two clients that participated in the trade
 */
struct AccountUpdate {
    std::string ticker;
    SIDE side;
    float price;
    float quantity;
    float capital_remaining;
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::ObUpdate> {
    using T = nutc::messages::ObUpdate;
    static constexpr auto value = object(
        "security",
        &T::ticker,
        "side",
        &T::side,
        "price",
        &T::price,
        "quantity",
        &T::quantity
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::AccountUpdate> {
    using T = nutc::messages::AccountUpdate;
    static constexpr auto value = object(
        "capital_remaining",
        &T::capital_remaining,
        "ticker",
        &T::ticker,
        "side",
        &T::side,
        "price",
        &T::price,
        "quantity",
        &T::quantity
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::Match> {
    using T = nutc::messages::Match;
    static constexpr auto value = object(
        "ticker",
        &T::ticker,
        "buyer_id",
        &T::buyer_id,
        "seller_id",
        &T::seller_id,
        "side",
        &T::side,
        "price",
        &T::price,
        "quantity",
        &T::quantity
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::StartTime> {
    using T = nutc::messages::StartTime;
    static constexpr auto value = object("start_time_ns", &T::start_time_ns);
};

/// \cond
template <>
struct glz::meta<nutc::messages::MarketOrder> {
    using T = nutc::messages::MarketOrder;
    static constexpr auto value = object(
        "client_id",
        &T::client_id,
        "side",
        &T::side,
        "ticker",
        &T::ticker,
        "quantity",
        &T::quantity,
        "price",
        &T::price
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::InitMessage> {
    using T = nutc::messages::InitMessage;
    static constexpr auto value =
        object("client_id", &T::client_id, "ready", &T::ready);
};
