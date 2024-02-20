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

struct MarketOrder {
    std::string client_id;
    SIDE side;
    std::string ticker;
    double quantity;
    double price;
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
