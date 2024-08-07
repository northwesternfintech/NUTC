#include "macros.hpp"

namespace nutc {
namespace test_utils {

bool
is_nearly_equal(double f_a, double f_b)
{
    double diff = std::fabs(f_a - f_b);
    return util::is_close_to_zero(diff);
}

bool
validate_match(
    const nutc::matching::stored_match& match, util::Ticker ticker,
    const std::string& buyer_id, const std::string& seller_id, util::Side side,
    double price, double quantity
)
{
    return match.position.ticker == ticker && match.buyer.get_id() == buyer_id
           && match.seller.get_id() == seller_id && match.position.side == side
           && is_nearly_equal(match.position.price, price)
           && is_nearly_equal(match.position.quantity, quantity);
}

bool
validate_ob_update(
    const util::position& update, util::Ticker ticker, util::Side side, double price,
    double quantity
)
{
    return update.ticker == ticker && update.side == side
           && is_nearly_equal(update.price, price)
           && is_nearly_equal(update.quantity, quantity);
}

bool
validate_limit_order(
    const limit_order& update, util::Ticker ticker, util::Side side, double price,
    double quantity
)
{
    return update.position.ticker == ticker && update.position.side == side
           && is_nearly_equal(update.position.price, price)
           && is_nearly_equal(update.position.quantity, quantity);
}

} // namespace test_utils
} // namespace nutc
