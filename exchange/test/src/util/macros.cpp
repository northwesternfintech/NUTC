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
    const nutc::messages::match& match, util::Ticker ticker,
    const std::string& buyer_id, const std::string& seller_id, util::Side side,
    double quantity, double price
)
{
    return match.position.ticker == ticker && match.buyer_id == buyer_id
           && match.seller_id == seller_id && match.position.side == side
           && match.position.price == price
           && is_nearly_equal(match.position.quantity, quantity);
}

bool
validate_ob_update(
    const util::position& update, util::Ticker ticker, util::Side side, double quantity,
    double price
)
{
    return update.ticker == ticker && update.side == side && update.price == price
           && is_nearly_equal(update.quantity, quantity);
}

bool
validate_limit_order(
    const limit_order& update, util::Ticker ticker, util::Side side, double quantity,
    double price
)
{
    return update.ticker == ticker && update.side == side && update.price == price
           && is_nearly_equal(update.quantity, quantity);
}

} // namespace test_utils
} // namespace nutc
