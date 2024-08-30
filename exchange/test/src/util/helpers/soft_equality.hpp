#pragma once

#include "common/messages_wrapper_to_exchange.hpp"

namespace nutc::test {
template <typename MessageT>
inline bool soft_equality(const MessageT& first, const MessageT& second);

template <>
inline bool
soft_equality(const common::limit_order& first, const common::limit_order& second)
{
    return first.ticker == second.ticker && first.side == second.side
           && first.quantity == second.quantity && first.price == second.price
           && first.ioc == second.ioc;
}

template <>
inline bool
soft_equality(const common::market_order& first, const common::market_order& second)
{
    return first.ticker == second.ticker && first.side == second.side
           && first.quantity == second.quantity;
}

template <>
inline bool
soft_equality(const common::cancel_order& first, const common::cancel_order& second)
{
    return first.order_id == second.order_id;
}
} // namespace nutc::test
