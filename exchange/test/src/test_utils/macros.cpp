#include "macros.hpp"

namespace nutc {
namespace testing_utils {
bool
is_nearly_equal(float f_a, float f_b, float epsilon)
{
    float abs_a = std::fabs(f_a);
    float abs_b = std::fabs(f_b);
    float diff = std::fabs(f_a - f_b);
    return diff <= ((abs_a < abs_b ? abs_b : abs_a) * epsilon);
}

bool
validate_match(
    const Match& match, const std::string& ticker, const std::string& buyer_id,
    const std::string& seller_id, messages::SIDE side, float price, float quantity
)
{
    return match.ticker == ticker && match.buyer_id == buyer_id
           && match.seller_id == seller_id && match.side == side
           && is_nearly_equal(match.price, price)
           && is_nearly_equal(match.quantity, quantity);
}

bool
validate_ob_update(
    const ObUpdate& update, const std::string& ticker, messages::SIDE side, float price,
    float quantity
)
{
    return update.ticker == ticker && update.side == side
           && is_nearly_equal(update.price, price)
           && is_nearly_equal(update.quantity, quantity);
}

bool
validate_market_order(
    const MarketOrder& update, const std::string& client_id, const std::string& ticker,
    messages::SIDE side, float price, float quantity
)
{
    return update.client_id == client_id && update.ticker == ticker
           && update.side == side && is_nearly_equal(update.price, price)
           && is_nearly_equal(update.quantity, quantity);
}

} // namespace testing_utils
} // namespace nutc
