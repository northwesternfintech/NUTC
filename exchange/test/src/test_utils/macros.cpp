#include "macros.hpp"

#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace test_utils {

stored_order
make_stored_order(market_order& order, traders::TraderContainer& manager)
{
    return stored_order{
        manager.get_trader(order.client_id),
        order.side,
        order.ticker,
        order.quantity,
        order.price,
        /*tick=*/0
    };
}

bool
is_nearly_equal(double f_a, double f_b, double epsilon)
{
    double abs_a = std::fabs(f_a);
    double abs_b = std::fabs(f_b);
    double diff = std::fabs(f_a - f_b);
    return diff <= ((abs_a < abs_b ? abs_b : abs_a) * epsilon);
}

bool
validate_match(
    const nutc::matching::stored_match& match, const std::string& ticker,
    const std::string& buyer_id, const std::string& seller_id, util::Side side,
    double price, double quantity
)
{
    return match.ticker == ticker && match.buyer->get_id() == buyer_id
           && match.seller->get_id() == seller_id && match.side == side
           && is_nearly_equal(match.price, price)
           && is_nearly_equal(match.quantity, quantity);
}

bool
validate_ob_update(
    const orderbook_update& update, const std::string& ticker, util::Side side,
    double price, double quantity
)
{
    return update.ticker == ticker && update.side == side
           && is_nearly_equal(update.price, price)
           && is_nearly_equal(update.quantity, quantity);
}

bool
validate_market_order(
    const market_order& update, const std::string& client_id, const std::string& ticker,
    util::Side side, double price, double quantity
)
{
    return update.client_id == client_id && update.ticker == ticker
           && update.side == side && is_nearly_equal(update.price, price)
           && is_nearly_equal(update.quantity, quantity);
}

} // namespace test_utils
} // namespace nutc
