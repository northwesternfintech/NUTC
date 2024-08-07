#include "order_storage.hpp"

namespace nutc {
namespace matching {
stored_order::stored_order(
    traders::GenericTrader& trader, util::Ticker ticker, util::Side side, double price,
    double quantity, bool ioc
) : limit_order{side, ticker, price, quantity, ioc}, trader(&trader)
{}

bool
stored_order::operator==(const stored_order& other) const
{
    return trader == other.trader && position == other.position;
}

} // namespace matching
} // namespace nutc
