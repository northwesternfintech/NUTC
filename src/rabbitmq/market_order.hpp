#pragma once

#include <glaze/glaze.hpp>

namespace nutc {
namespace rabbitmq {

struct MarketOrder {
    std::string security;
    int quantity;
    bool side;
    std::string type;
};
} // namespace rabbitmq
} // namespace nutc

template <>
struct glz::meta<nutc::rabbitmq::MarketOrder> {
    using T = nutc::rabbitmq::MarketOrder;
    static constexpr auto value = object(
        "security",
        &T::security,
        "quantity",
        &T::quantity,
        "side",
        &T::side,
        "type",
        &T::type
    );
};
