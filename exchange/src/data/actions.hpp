#pragma once

#include <string>
#include <glaze/glaze.hpp>

namespace nutc {
namespace actions {

struct Match {
    std::string buyer_id;
    std::string seller_id;
    std::string ticker;
    double quantity;
    double price;
    long timestamp;
};

struct OrderbookUpdate {
    std::string trader_id;
    std::string ticker;
    std::string side;
    double quantity;
    double price;
    long timestamp;
};

struct StockPriceUpdate {
    std::string ticker;
    double highest_bid;
    double lowest_ask;
};

} // namespace messages
} // namespace nutc

template <>
struct glz::meta<nutc::messages::Match> {
    using T = nutc::messages::Match;
    static constexpr auto value = glz::object(
        "buyer_id", &T::buyer_id, "seller_id", &T::seller_id, "ticker", &T::ticker,
        "quantity", &T::quantity, "price", &T::price, "timestamp", &T::timestamp
    );
};

template <>
struct glz::meta<nutc::messages::OrderbookUpdate> {
    using T = nutc::messages::OrderbookUpdate;
    static constexpr auto value = glz::object(
        "trader_id", &T::trader_id, "ticker", &T::ticker, "side", &T::side,
        "quantity", &T::quantity, "price", &T::price, "timestamp", &T::timestamp
    );
};

template <>
struct glz::meta<nutc::messages::StockPriceUpdate> {
    using T = nutc::messages::StockPriceUpdate;
    static constexpr auto value = glz::object(
        "ticker", &T::ticker, "highest_bid", &T::highest_bid, "lowest_ask",
        &T::lowest_ask
    );
};