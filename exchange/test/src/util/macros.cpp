#include "macros.hpp"

namespace nutc::test {

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
