#include "macros.hpp"

namespace nutc::common {
void
PrintTo(const AlgoLanguage& op, std::ostream* os)
{
    switch (op) {
        case AlgoLanguage::cpp:
            *os << "CPP";
            break;
        case AlgoLanguage::python:
            *os << "PYTHON";
            break;
        default:
            *os << "UNKNOWN_LANGUAGE";
            break;
    }
}
} // namespace nutc::common

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
validate_match(
    const nutc::exchange::tagged_match& match, common::Ticker ticker,
    const std::string& buyer_id, const std::string& seller_id, common::Side side,
    double quantity, double price
)
{
    return match.ticker == ticker && match.buyer->get_id() == buyer_id
           && match.seller->get_id() == seller_id && match.side == side
           && match.price == price && match.quantity == quantity;
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
