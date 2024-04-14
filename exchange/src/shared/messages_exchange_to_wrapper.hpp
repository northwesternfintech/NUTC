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
    int64_t start_time_ns;

    StartTime() = default;

    StartTime(int64_t stns) : start_time_ns(stns) {}
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
    double buyer_capital;
    double seller_capital;

    Match() = default;

    Match(
        std::string ticker, SIDE side, double price, double quantity, std::string bid,
        std::string sid, double bcap, double scap
    ) :
        ticker(ticker),
        side(side), price(price), quantity(quantity), buyer_id(bid), seller_id(sid),
        buyer_capital(bcap), seller_capital(scap)
    {}
};

/**
 * @brief Sent by exchange to clients to indicate an orderbook update
 */
struct ObUpdate {
    std::string ticker;
    SIDE side;
    double price;
    double quantity;

    ObUpdate() = default;

    ObUpdate(std::string ticker, SIDE side, double price, double quantity) :
        ticker(ticker), side(side), price(price), quantity(quantity)
    {}

    bool
    operator==(const ObUpdate& other) const
    {
        return ticker == other.ticker && side == other.side && price == other.price
               && quantity == other.quantity;
    }
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
struct glz::meta<nutc::messages::Match> {
    using T = nutc::messages::Match;
    static constexpr auto value = object(
        "ticker", &T::ticker, "buyer_id", &T::buyer_id, "seller_id", &T::seller_id,
        "side", &T::side, "price", &T::price, "quantity", &T::quantity, "buyer_capital",
        &T::buyer_capital, "seller_capital", &T::seller_capital
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::StartTime> {
    using T = nutc::messages::StartTime;
    static constexpr auto value = object("start_time_ns", &T::start_time_ns);
};
