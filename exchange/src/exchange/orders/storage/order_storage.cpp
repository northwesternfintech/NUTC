#include "order_storage.hpp"

namespace nutc {
namespace matching {
stored_order::stored_order(
    traders::GenericTrader& trader, util::Ticker ticker, util::Side side, double price,
    double quantity, bool ioc
) :
    trader(trader), ticker(ticker), side(side), price(price), quantity(quantity),
    ioc(ioc), order_index(get_and_increment_global_index())
{}

bool
stored_order::operator==(const stored_order& other) const
{
    return &trader == &other.trader && ticker == other.ticker && side == other.side
           && util::is_close_to_zero(price - other.price)
           && util::is_close_to_zero(quantity - other.quantity) && ioc == other.ioc;
}

} // namespace matching
} // namespace nutc
