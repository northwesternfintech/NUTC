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

struct MarketOrder {
    std::string client_uid;
    SIDE side;
    std::string type;
    std::string ticker;
    float quantity;
    float price;
};

struct ObUpdate {
    std::string security;
    float price;
    float quantity;
};

} // namespace rabbitmq
} // namespace nutc

template <>
struct glz::meta<nutc::messages::ObUpdate> {
    using T = nutc::messages::ObUpdate;
    static constexpr auto value =
        object("security", &T::security, "price", &T::price, "quantity", &T::quantity);
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
        "client_uid",
        &T::client_uid,
        "side",
        &T::side,
        "type",
        &T::type,
        "ticker",
        &T::ticker,
        "quantity",
        &T::quantity,
        "price",
        &T::price
    );
};

template <>
struct glz::meta<nutc::messages::InitMessage> {
    using T = nutc::messages::InitMessage;
    static constexpr auto value =
        object("client_uid", &T::client_uid, "ready", &T::ready);
};
