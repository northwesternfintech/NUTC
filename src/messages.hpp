#pragma once

#include <glaze/glaze.hpp>

namespace nutc {
namespace messages {

enum SIDE { BUY, SELL };

struct ShutdownMessage {
    std::string shutdown_reason;
};

struct RMQError {
    std::string message; // todo: make enum?
};

struct InitMessage {
    std::string client_uid;
    bool ready;
};

struct Match {
    std::string ticker;
    std::string buyer_uid;
    std::string seller_uid;
    float price;
    float quantity;
};

struct MarketOrder {
    std::string client_uid;
    SIDE side;
    std::string type;
    std::string ticker;
    float quantity;
    float price;

    bool
    operator<(const MarketOrder& other) const
    {
        // assuming both sides are same
        // otherwise, this shouldn't even be called
        if (this->side == BUY) {
            return this->price > other.price;
        }
        else {
            return this->price < other.price;
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
        if (this->side == BUY && this->price < other.price) {
            return false;
        }
        if (this->side == SELL && this->price > other.price) {
            return false;
        }
        return true;
    }
};

struct ObUpdate {
    std::string security;
    float price;
    float quantity;
};

} // namespace messages
} // namespace nutc

template <>
struct glz::meta<nutc::messages::ObUpdate> {
    using T = nutc::messages::ObUpdate;
    static constexpr auto value =
        object("security", &T::security, "price", &T::price, "quantity", &T::quantity);
};

template <>
struct glz::meta<nutc::messages::Match> {
    using T = nutc::messages::Match;
    static constexpr auto value = object(
        "ticker", &T::ticker, "buyer_uid", &T::buyer_uid, "seller_uid", &T::seller_uid,
        "price", &T::price, "quantity", &T::quantity
    );
};

template <>
struct glz::meta<nutc::messages::ShutdownMessage> {
    using T = nutc::messages::ShutdownMessage;
    static constexpr auto value = object("shutdown_reason", &T::shutdown_reason);
};

template <>
struct glz::meta<nutc::messages::MarketOrder> {
    using T = nutc::messages::MarketOrder;
    static constexpr auto value = object(
        "client_uid", &T::client_uid, "side", &T::side, "type", &T::type, "ticker",
        &T::ticker, "quantity", &T::quantity, "price", &T::price
    );
};

template <>
struct glz::meta<nutc::messages::InitMessage> {
    using T = nutc::messages::InitMessage;
    static constexpr auto value =
        object("client_uid", &T::client_uid, "ready", &T::ready);
};
