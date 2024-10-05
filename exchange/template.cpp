// this is the server-side part of the cpp algorithm template implementation
// it will include the user submitted hpp file and then be compiled as a shared
// library

// #include "template.hpp"

#include <functional>
#include <string>
#include <cstdint>

using PlaceMarketOrder = std::function<bool(Side, Ticker, float)>;
using PlaceLimitOrder = std::function<std::int64_t(Side, Ticker, float, float, bool)>;
using CancelOrder = std::function<bool(Ticker, std::int64_t order_id)>;

static PlaceMarketOrder s_place_market_order;
static PlaceLimitOrder s_place_limit_order;
static CancelOrder s_cancel_order;

bool
place_market_order(Side side, Ticker ticker, float quantity)
{
    return s_place_market_order(side, ticker, quantity);
}

std::int64_t
place_limit_order(Side side, Ticker ticker, float quantity, float price, bool ioc)
{
    return s_place_limit_order(side, ticker, quantity, price, ioc);
}

bool
cancel_order(Ticker ticker, std::int64_t order_id)
{
    return s_cancel_order(ticker, order_id);
}

extern "C" {
Strategy*
init(
    PlaceMarketOrder place_market_order, PlaceLimitOrder place_limit_order,
    CancelOrder cancel_order
)
{
    s_place_market_order = std::move(place_market_order);
    s_place_limit_order = std::move(place_limit_order);
    s_cancel_order = std::move(cancel_order);
    return new Strategy();
}

void
on_trade_update(
    Strategy* strategy, Ticker ticker, Side side, float quantity, float price
)
{
    strategy->on_trade_update(ticker, side, quantity, price);
}

void
on_orderbook_update(
    Strategy* strategy, Ticker ticker, Side side, float quantity, float price
)
{
    strategy->on_orderbook_update(ticker, side, quantity, price);
}

void
on_account_update(
    Strategy* strategy, Ticker ticker, Side side, float price, float quantity,
    float capital_remaining
)
{
    strategy->on_account_update(ticker, side, price, quantity, capital_remaining);
}
}
