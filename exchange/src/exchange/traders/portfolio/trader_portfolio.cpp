#include "trader_portfolio.hpp"

#include <fmt/core.h>

#include <iostream>

namespace nutc::exchange {
TraderPortfolio::TraderPortfolio(common::decimal_price initial_capital) :
    initial_capital_{initial_capital}
{}

common::decimal_price
TraderPortfolio::compute_capital_tolerance() const
{
    // TODO: FIX ULTRA HIGH PRIO
    return (common::decimal_price{1.0} - get_capital_utilization())
           * (get_initial_capital());
}

// todo: pass these by ref
void
TraderPortfolio::notify_match(common::position match)
{
    common::decimal_price total_cap = match.price * match.quantity;
    if (match.side == common::Side::buy) {
        modify_holdings(match.ticker, match.quantity);
        modify_capital(total_cap * -1.0);
    }
    else {
        modify_holdings(match.ticker, -(match.quantity));
        modify_capital(total_cap);
    }
}

void
TraderPortfolio::notify_position_change(common::position order)
{
    decimal_high_precision notional_value =
        order.price.high_precision_multiply(order.quantity);

    if (order.side == common::Side::buy) {
        open_bids_ += order.quantity;
        long_interest_ += notional_value;
    }
    else {
        open_asks_ += order.quantity;
        short_interest_ += notional_value;
    }
}

void
TraderPortfolio::modify_capital(common::decimal_price change_in_capital)
{
    capital_delta_ += decimal_high_precision{change_in_capital};
}

common::decimal_price
TraderPortfolio::get_capital_delta() const
{
    return common::decimal_price{capital_delta_};
}

common::decimal_price
TraderPortfolio::get_long_interest() const
{
    return common::decimal_price{long_interest_};
}

common::decimal_price
TraderPortfolio::get_short_interest() const
{
    return common::decimal_price{short_interest_};
}

common::decimal_price
TraderPortfolio::get_capital_utilization() const
{
    common::decimal_price capital_util =
        (get_long_interest() + get_short_interest()) / get_initial_capital();
    assert(0 <= capital_util && capital_util <= 1.0);
    return capital_util;
}

common::decimal_price
TraderPortfolio::get_initial_capital() const
{
    return initial_capital_;
}

common::decimal_quantity
TraderPortfolio::get_holdings(common::Ticker ticker) const
{
    auto ticker_index = std::to_underlying(ticker);
    assert(0 <= ticker_index && ticker_index < holdings_.size());
    return holdings_[ticker_index];
}

common::decimal_quantity
TraderPortfolio::modify_holdings(
    common::Ticker ticker, common::decimal_quantity change_in_holdings
)
{
    auto ticker_index = std::to_underlying(ticker);
    assert(0 <= ticker_index && ticker_index < holdings_.size());
    return holdings_[ticker_index] += change_in_holdings;
}

common::decimal_price
TraderPortfolio::get_capital() const
{
    return initial_capital_ + common::decimal_price{capital_delta_};
}

common::decimal_quantity
TraderPortfolio::get_open_bids() const
{
    return open_bids_;
}

common::decimal_quantity
TraderPortfolio::get_open_asks() const
{
    return open_asks_;
}

} // namespace nutc::exchange
