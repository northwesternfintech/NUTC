#include "macros.hpp"

#include "exchange/traders/trader_manager.hpp"
#include "exchange/traders/trader_types.hpp"

#include <iostream>

namespace nutc {
namespace testing_utils {
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
    const Match& match, const std::string& ticker, const std::string& buyer_id,
    const std::string& seller_id, messages::SIDE side, double price, double quantity
)
{
    return match.ticker == ticker && match.buyer_id == buyer_id
           && match.seller_id == seller_id && match.side == side
           && is_nearly_equal(match.price, price)
           && is_nearly_equal(match.quantity, quantity);
}

bool
validate_ob_update(
    const ObUpdate& update, const std::string& ticker, messages::SIDE side, double price,
    double quantity
)
{
    return update.ticker == ticker && update.side == side
           && is_nearly_equal(update.price, price)
           && is_nearly_equal(update.quantity, quantity);
}

bool
validate_market_order(
    const MarketOrder& update, const std::string& client_id, const std::string& ticker,
    messages::SIDE side, double price, double quantity
)
{
    return update.client_id == client_id && update.ticker == ticker
           && update.side == side && is_nearly_equal(update.price, price)
           && is_nearly_equal(update.quantity, quantity);
}

void
add_client_simple(
    manager::ClientManager& manager, const std::string& client_id, double capital
)
{
    std::string trader_id =
        manager.add_client(manager::local_trader_t{client_id, client_id});
    std::visit(
        [&](auto&& res) { res.set_capital(capital); }, manager.get_client(trader_id)
    );
    assert(trader_id == client_id);
}

void
modify_holdings_simple(
    manager::ClientManager& manager, const std::string& client_id,
    const std::string& ticker, double quantity
)
{
    auto& client = manager.get_client(client_id);
    std::visit(
        [&ticker, &quantity](auto&& client) {
            client.modify_holdings(ticker, quantity);
        },
        client
    );
}

void
modify_capital_simple(
    manager::ClientManager& manager, const std::string& client_id, double capital_change
)
{
    auto& client = manager.get_client(client_id);
    std::visit(
        [&capital_change](auto&& client) { client.modify_capital(capital_change); },
        client
    );
}

double
get_capital_simple(manager::ClientManager& manager, const std::string& client_id)
{
    auto& client = manager.get_client(client_id);
    return std::visit([](auto&& client) { return client.get_capital(); }, client);
}

} // namespace testing_utils
} // namespace nutc
