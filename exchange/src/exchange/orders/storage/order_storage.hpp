#pragma once
#include "common/messages_wrapper_to_exchange.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"

#include <fmt/format.h>

namespace nutc::exchange {

template <typename BaseOrderT>
class tagged_order : public BaseOrderT {
public:
    GenericTrader* trader;

    tagged_order(GenericTrader& order_creator, const auto& order) :
        BaseOrderT(order), trader(&order_creator)
    {}

    template <typename... Args>
    tagged_order(GenericTrader& order_creator, Args&&... args)
    requires std::is_constructible_v<BaseOrderT, Args...>
        : BaseOrderT(args...), trader(&order_creator)
    {}

    bool operator==(const tagged_order& other) const = default;
};

using tagged_limit_order = tagged_order<common::limit_order>;
using tagged_market_order = tagged_order<common::market_order>;

using OrderVariant =
    std::variant<common::cancel_order, tagged_limit_order, tagged_market_order>;

template <typename T>
inline constexpr bool is_limit_order_v =
    std::is_same_v<std::remove_cvref_t<T>, tagged_limit_order>;

template <typename T>
inline constexpr bool is_market_order_v =
    std::is_same_v<std::remove_cvref_t<T>, tagged_market_order>;

template <typename T>
concept TaggedOrder = is_limit_order_v<T> || is_market_order_v<T>;

} // namespace nutc::exchange
