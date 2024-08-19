#include "order_storage.hpp"

namespace nutc {
namespace matching {
stored_order::stored_order(
    traders::GenericTrader& trader, util::Ticker ticker, util::Side side,
    double quantity, util::decimal_price price, bool ioc
) : limit_order{side, ticker, quantity, price, ioc}, trader(&trader)
{}

bool
stored_order::operator==(const stored_order& other) const
{
    return trader == other.trader && position == other.position;
}

} // namespace matching
} // namespace nutc
