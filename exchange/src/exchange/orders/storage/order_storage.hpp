#pragma once
#include "exchange/traders/trader_types/generic_trader.hpp"
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

struct stored_order : public messages::limit_order {
    traders::GenericTrader* trader;
    bool was_removed{false};
    uint64_t order_index = get_and_increment_global_index();

    inline static uint64_t
    get_and_increment_global_index()
    {
        static uint64_t global_index = 0;
        return global_index++;
    }

    operator util::position() const { return position; }

    stored_order(
        traders::GenericTrader& trader, util::Ticker ticker, util::Side side,
        double quantity, double price, bool ioc = false
    );

    stored_order(const stored_order& other) = default;

    ~stored_order() = default;
    bool operator==(const stored_order& other) const;
};
} // namespace matching
} // namespace nutc
