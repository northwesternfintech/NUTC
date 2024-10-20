#pragma once
#include "common/messages_wrapper_to_exchange.hpp"
#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"

#include <fmt/format.h>

namespace nutc::exchange {

// TODO: make generic again
class tagged_limit_order : public common::limit_order {
public:
    GenericTrader* trader;

    tagged_limit_order(GenericTrader& order_creator, const limit_order& order) :
        limit_order(order), trader(&order_creator)
    {}

    tagged_limit_order(
        GenericTrader& order_creator, common::Ticker ticker, common::Side side,
        common::decimal_quantity decimal_quantity, common::decimal_price decimal_price,
        bool ioc = false
    ) :
        limit_order(
            ticker, side, decimal_quantity, decimal_price, ioc, common::get_time(),
            common::generate_order_id()
        ),
        trader(&order_creator)
    {}

    bool operator==(const tagged_limit_order& other) const = default;
};

// TODO: make generic again
class tagged_market_order : public common::market_order {
public:
    GenericTrader* trader;

    tagged_market_order(GenericTrader& order_creator, const market_order& order) :
        market_order(order), trader(&order_creator)
    {}

    tagged_market_order(
        GenericTrader& order_creator, common::Ticker ticker, common::Side side,
        common::decimal_quantity decimal_quantity
    ) :
        market_order(ticker, side, decimal_quantity, common::get_time()),
        trader(&order_creator)
    {}

    bool operator==(const tagged_market_order& other) const = default;
};

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
