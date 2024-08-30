#include "macros.hpp"

namespace nutc::test {
bool
order_equality(const common::market_order& order1, const common::market_order& order2)
{
    return order1.ticker == order2.ticker && order1.side == order2.side
           && order1.quantity == order2.quantity;
}

bool
order_equality(const limit_order& order1, const limit_order& order2)
{
    return order1.ticker == order2.ticker && order1.side == order2.side
           && order1.quantity == order2.quantity && order1.price == order2.price
           && order1.ioc == order2.ioc;
}

bool
is_nearly_equal(double f_a, double f_b)
{
    double diff = std::fabs(f_a - f_b);
    return common::is_close_to_zero(diff);
}

bool
validate_match(
    const nutc::common::match& match, common::Ticker ticker,
    const std::string& buyer_id, const std::string& seller_id, common::Side side,
    double quantity, double price
)
{
    return match.position.ticker == ticker && match.buyer_id == buyer_id
           && match.seller_id == seller_id && match.position.side == side
           && match.position.price == price && match.position.quantity == quantity;
}

bool
validate_ob_update(
    const common::position& update, common::Ticker ticker, common::Side side,
    double quantity, double price
)
{
    return update.ticker == ticker && update.side == side && update.price == price
           && update.quantity == quantity;
}

bool
validate_limit_order(
    const limit_order& update, common::Ticker ticker, common::Side side,
    double quantity, double price
)
{
    return update.ticker == ticker && update.side == side && update.price == price
           && update.quantity == quantity;
}

} // namespace nutc::test
