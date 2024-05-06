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
struct init_message {
    std::string client_id;
    bool ready;
};

struct market_order {
    std::string client_id;
    util::Side side;
    std::string ticker;
    double quantity;
    double price;
};

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::market_order> {
    using t = nutc::messages::market_order;
    static constexpr auto value = object( // NOLINT
        "trader_id", &t::client_id, "ticker", &t::ticker, "side", &t::side, "quantity",
        &t::quantity, "price", &t::price
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::init_message> {
    using t = nutc::messages::init_message;
    static constexpr auto value = // NOLINT
        object("trader_id", &t::client_id, "ready", &t::ready);
};
