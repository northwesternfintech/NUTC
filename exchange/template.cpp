#include "template.hpp"

Strategy::Strategy(MarketOrderFunc place_mo) : place_market_order(place_mo)
{
    // Your initialization code goes here.
}

Strategy::~Strategy() {}

void
Strategy::on_trade_update(
    std::string ticker, std::string side, double price, double quantity
)
{}

void
Strategy::on_orderbook_update(
    std::string ticker, std::string side, double price, double quantity
)
{}

void
Strategy::on_account_update(
    std::string ticker, std::string side, double price, double quantity,
    double buyer_capital
)
{}

// DO NOT MODIFY
extern "C" {
void
on_trade_update(
    Strategy* strategy, std::string ticker, std::string side, double price,
    double quantity
)
{
    strategy->on_trade_update(ticker, side, price, quantity);
}

void
on_orderbook_update(
    Strategy* strategy, std::string ticker, std::string side, double price,
    double quantity
)
{
    strategy->on_orderbook_update(ticker, side, price, quantity);
}

void
on_account_update(
    Strategy* strategy, std::string ticker, std::string side, double price,
    double quantity, double buyer_capital
)
{
    strategy->on_account_update(ticker, side, price, quantity, buyer_capital);
}

Strategy*
create_strategy(MarketOrderFunc cb)
{
    return new Strategy(cb);
}

void
delete_strategy(Strategy* strategy)
{
    delete strategy;
}
}
