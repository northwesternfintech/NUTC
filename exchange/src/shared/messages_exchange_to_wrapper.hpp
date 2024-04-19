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

struct start_time {
    int64_t start_time_ns{};

    start_time() = default;

    explicit start_time(int64_t stns) : start_time_ns(stns) {}
};

/**
 * @brief Sent by exchange to a client to indicate a match has occurred
 */
struct match {
    std::string ticker{};
    util::Side side;
    double price{};
    double quantity{};
    std::string buyer_id{};
    std::string seller_id{};
    double buyer_capital{};
    double seller_capital{};

    match() = default;

    match(
        std::string ticker, util::Side side, double price, double quantity,
        std::string bid, std::string sid, double bcap, double scap
    ) :
        ticker(std::move(ticker)),
        side(side), price(price), quantity(quantity), buyer_id(std::move(bid)),
        seller_id(std::move(sid)), buyer_capital(bcap), seller_capital(scap)
    {}
};

/**
 * @brief Sent by exchange to clients to indicate an orderbook update
 */
struct orderbook_update {
    std::string ticker{};
    util::Side side{};
    double price{};
    double quantity{};

    orderbook_update() = default;

    orderbook_update(
        std::string ticker, util::Side side, double price, double quantity
    ) :
        ticker(std::move(ticker)),
        side(side), price(price), quantity(quantity)
    {}

    bool operator==(const orderbook_update& other) const = default;
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::orderbook_update> {
    using t = nutc::messages::orderbook_update;
    static constexpr auto value = object( // NOLINT
        "security", &t::ticker, "side", &t::side, "price", &t::price, "quantity",
        &t::quantity
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::match> {
    using t = nutc::messages::match;
    static constexpr auto value = object( // NOLINT
        "ticker", &t::ticker, "buyer_id", &t::buyer_id, "seller_id", &t::seller_id,
        "side", &t::side, "price", &t::price, "quantity", &t::quantity, "buyer_capital",
        &t::buyer_capital, "seller_capital", &t::seller_capital
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::start_time> {
    using t = nutc::messages::start_time;
    static constexpr auto value = // NOLINT
        object("start_time_ns", &t::start_time_ns);
};
