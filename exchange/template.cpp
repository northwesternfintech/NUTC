// this is the server-side part of the cpp algorithm template implementation
// it will include the user submitted hpp file and then be compiled as a shared
// library

#include "template.hpp"

#include <functional>

using PlaceMarketOrder = std::function<bool(std::string const&, std::string const&, double)>;
using PlaceLimitOrder = std::function<std::int64_t(std::string const&, std::string const&, double, double, bool)>;
using CancelOrder = std::function<bool(std::string const& ticker, std::int64_t order_id)>;

static PlaceMarketOrder s_place_market_order;
static PlaceLimitOrder s_place_limit_order;
static CancelOrder s_cancel_order;

bool place_market_order(std::string const& side, std::string const& ticker, double quantity) {
    return s_place_market_order(side, ticker, quantity);
}

std::int64_t place_limit_order(
    std::string const& side, std::string const& ticker, double quantity, double price,
    bool ioc) {
    return s_place_limit_order(side, ticker, quantity, price, ioc);
}

bool cancel_order(std::string const& ticker, std::int64_t order_id) {
    return s_cancel_order(ticker, order_id);
}

extern "C" {
Strategy*
init(PlaceMarketOrder place_market_order, PlaceLimitOrder place_limit_order, CancelOrder cancel_order) {
    s_place_market_order = std::move(place_market_order);
    s_place_limit_order = std::move(place_limit_order);
    s_cancel_order = std::move(cancel_order);
    return new Strategy();
}

void on_trade_update(
    Strategy* strategy, std::string const& ticker, std::string const& side,
    double quantity, double price) {
    strategy->on_trade_update(ticker, side, quantity, price);
}

void on_orderbook_update(
    Strategy* strategy, std::string const& ticker, std::string const& side,
    double quantity, double price) {
    strategy->on_orderbook_update(ticker, side, quantity, price);
}

void on_account_update(
    Strategy* strategy, std::string const& ticker, std::string const& side,
    double price, double quantity, double capital_remaining) {
    strategy->on_account_update(ticker, side, price, quantity, capital_remaining);
}
}
