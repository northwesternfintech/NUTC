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
 * @brief Sent by the exchange to initiate client shutdowns
 */
struct ShutdownMessage {
    std::string shutdown_reason;
};

/**
 * @brief Returned by functions to indicate an issue with RMQ communication
 */
struct RMQError {
    std::string message; // todo: make enum?
};

/**
 * @brief Sent by clients to the exchange to indicate they're initialized and may or may
 * not be participating in the competition
 */
struct InitMessage {
    std::string client_uid;
    bool ready;
};

/**
 * @brief Sent by exchange to a client to indicate a match has occured
 */
struct Match {
    std::string ticker;
    std::string buyer_uid;
    std::string seller_uid;
    SIDE side;
    float price;
    float quantity;
};

/**
 * @brief Sent by clients to the exchange to place an order
 * TODO: client_uid=="SIMULATED" indicates simulated order with no actual
 * owner, but this is improper. Instead, it should be an optional
 */
struct MarketOrder {
    std::string client_uid;
    SIDE side;
    std::string type;
    std::string ticker;
    float quantity;
    float price;

    // Used to sort orders by time created
    long long order_index;
    static long long global_index;

    MarketOrder() { order_index = global_index++; }

    MarketOrder(
        const std::string& client_uid, SIDE side, const std::string& type,
        const std::string& ticker, float quantity, float price
    ) :
        client_uid(client_uid),
        side(side), type(type), ticker(ticker), quantity(quantity), price(price)
    {
        order_index = global_index++;
    }

    // toString
    std::string
    to_string() const
    {
        std::string side_str = side == SIDE::BUY ? "BUY" : "SELL";
        return fmt::format(
            "MarketOrder(client_uid={}, side={}, type={}, ticker={}, quantity={}, "
            "price={})",
            client_uid, side_str, type, ticker, quantity, price
        );
    }

    bool
    operator<(const MarketOrder& other) const
    {
        // assuming both sides are same
        // otherwise, this shouldn't even be called
        if (this->side == SIDE::BUY) {
            return this->price < other.price;
        }
        else {
            return this->price > other.price;
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
};

/**
 * @brief Sent by exchange to clients to indicate an orderbook update
 */
struct ObUpdate {
    std::string security;
    SIDE side;
    float price;
    float quantity;
};

/**
 * @brief Sent by exchange to clients to indicate an update with their specific account
 * This is only sent to the two clients that participated in the trade
 */
struct AccountUpdate {
    float capital_remaining;
    std::string ticker;
    SIDE side;
    float price;
    float quantity;
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::ObUpdate> {
    using T = nutc::messages::ObUpdate;
    static constexpr auto value = object(
        "security", &T::security, "side", &T::side, "price", &T::price, "quantity",
        &T::quantity
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::AccountUpdate> {
    using T = nutc::messages::AccountUpdate;
    static constexpr auto value = object(
        "capital_remaining", &T::capital_remaining, "ticker", &T::ticker, "side",
        &T::side, "price", &T::price, "quantity", &T::quantity
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::Match> {
    using T = nutc::messages::Match;
    static constexpr auto value = object(
        "ticker", &T::ticker, "buyer_uid", &T::buyer_uid, "seller_uid", &T::seller_uid,
        "side", &T::side, "price", &T::price, "quantity", &T::quantity
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::ShutdownMessage> {
    using T = nutc::messages::ShutdownMessage;
    static constexpr auto value = object("shutdown_reason", &T::shutdown_reason);
};

/// \cond
template <>
struct glz::meta<nutc::messages::MarketOrder> {
    using T = nutc::messages::MarketOrder;
    static constexpr auto value = object(
        "client_uid", &T::client_uid, "side", &T::side, "type", &T::type, "ticker",
        &T::ticker, "quantity", &T::quantity, "price", &T::price
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::InitMessage> {
    using T = nutc::messages::InitMessage;
    static constexpr auto value =
        object("client_uid", &T::client_uid, "ready", &T::ready);
};
