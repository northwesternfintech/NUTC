#pragma once

#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>

namespace nutc {

/**
 * @brief Contains all types used by glaze and the exchange for orders, matching,
 * communication, etc
 */
namespace messages {

struct StartTime {
    uint64_t start_time_ns;
};

/**
 * @brief Sent by exchange to a client to indicate a match has occurred
 */
struct Match {
    std::string ticker;
    SIDE side;
    double price;
    double quantity;
    std::string buyer_id;
    std::string seller_id;
};

/**
 * @brief Sent by exchange to clients to indicate an orderbook update
 */
struct ObUpdate {
    std::string ticker;
    SIDE side;
    double price;
    double quantity;
};

/**
 * @brief Sent by exchange to clients to indicate an update with their specific account
 * This is only sent to the two clients that participated in the trade
 */
struct AccountUpdate {
    std::string ticker;
    SIDE side;
    double price;
    double quantity;
    double capital_remaining;
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::ObUpdate> {
    using T = nutc::messages::ObUpdate;
    static constexpr auto value = object(
        "security", &T::ticker, "side", &T::side, "price", &T::price, "quantity",
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
        "ticker", &T::ticker, "buyer_id", &T::buyer_id, "seller_id", &T::seller_id,
        "side", &T::side, "price", &T::price, "quantity", &T::quantity
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::StartTime> {
    using T = nutc::messages::StartTime;
    static constexpr auto value = object("start_time_ns", &T::start_time_ns);
};
