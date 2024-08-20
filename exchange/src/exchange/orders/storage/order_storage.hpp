#pragma once
#include "exchange/traders/trader_types/generic_trader.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <fmt/format.h>

namespace nutc {
namespace matching {

struct stored_match {
    traders::GenericTrader& buyer;
    traders::GenericTrader& seller;
    util::position position;

    operator messages::match() const
    {
        return {
            position, buyer.get_id(), seller.get_id(), buyer.get_capital(),
            seller.get_capital()
        };
    }
};

template <typename BaseOrderT>
struct tagged_order : public BaseOrderT {
    traders::GenericTrader* trader;
    bool was_removed{false};
    uint64_t order_index = get_and_increment_global_index();

    inline static uint64_t
    get_and_increment_global_index()
    {
        static uint64_t global_index = 0;
        return global_index++;
    }

    tagged_order(traders::GenericTrader& order_creator, const auto& order) :
        BaseOrderT(order), trader(&order_creator)
    {}

    template <typename... Args>
    tagged_order(traders::GenericTrader& order_creator, Args&&... args)
    requires std::is_constructible_v<BaseOrderT, Args...>
        : BaseOrderT(args...), trader(&order_creator)
    {}

    operator util::position() const { return BaseOrderT::position; }

    bool operator==(const tagged_order& other) const = default;
};

using tagged_limit_order = tagged_order<messages::limit_order>;
using tagged_market_order = tagged_order<messages::market_order>;

} // namespace matching
} // namespace nutc
