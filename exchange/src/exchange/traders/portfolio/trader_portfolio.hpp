#pragma once

#include "common/types/decimal.hpp"
#include "common/types/position.hpp"
#include "common/types/ticker.hpp"

namespace nutc::exchange {
class TraderPortfolio {
    using decimal_price = common::decimal_price;
    using decimal_quantity = common::decimal_quantity;
    using decimal_high_precision = common::decimal_high_precision;

    decimal_price initial_capital_;
    decimal_quantity open_bids_;
    decimal_quantity open_asks_;
    decimal_high_precision capital_delta_;
    decimal_high_precision short_interest_;
    decimal_high_precision long_interest_;
    std::array<decimal_quantity, common::TICKERS.size()> holdings_{};

public:
    explicit TraderPortfolio(decimal_price initial_capital);

    common::decimal_price compute_capital_tolerance() const;

    void notify_match(common::position match);

    void notify_position_change(common::position order);

    void modify_capital(decimal_price change_in_capital);

    decimal_price get_capital_delta() const;

    decimal_price get_long_interest() const;

    decimal_price get_short_interest() const;

    decimal_price get_capital_utilization() const;

    decimal_price get_initial_capital() const;

    decimal_quantity get_holdings(common::Ticker ticker) const;

    decimal_quantity
    modify_holdings(common::Ticker ticker, decimal_quantity change_in_holdings);

    decimal_price get_capital() const;

    decimal_quantity get_open_bids() const;

    decimal_quantity get_open_asks() const;

    void modify_open_bids(decimal_quantity delta);

    void modify_open_asks(decimal_quantity delta);
};
} // namespace nutc::exchange
