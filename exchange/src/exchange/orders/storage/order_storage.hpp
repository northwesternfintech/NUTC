#pragma once
#include "exchange/traders/trader_types/generic_trader.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <fmt/format.h>

namespace nutc::exchange {

template <typename BaseOrderT>
class tagged_order : public BaseOrderT {
    inline static constinit std::uint64_t global_index = 0;

public:
    GenericTrader* trader;
    bool active{true};
    uint64_t order_index{++global_index};

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

using tagged_limit_order = tagged_order<shared::timed_limit_order>;
using tagged_market_order = tagged_order<shared::timed_market_order>;

template <typename T>
struct is_limit_order : std::false_type {};

template <>
struct is_limit_order<tagged_limit_order> : std::true_type {};

template <typename T>
inline constexpr bool is_limit_order_v = is_limit_order<std::remove_cvref_t<T>>::value;

template <typename T>
struct is_market_order : std::false_type {};

template <>
struct is_market_order<tagged_market_order> : std::true_type {};

template <typename T>
inline constexpr bool is_market_order_v =
    is_market_order<std::remove_cvref_t<T>>::value;

template <typename T>
concept TaggedOrder = is_limit_order_v<T> || is_market_order_v<T>;

} // namespace nutc::exchange
