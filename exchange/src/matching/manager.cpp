#include "matching/manager.hpp"

namespace nutc {
namespace engine_manager {
std::optional<std::reference_wrapper<Engine>>
Manager::getEngine(const std::string& ticker)
{
    if (engines.find(ticker) != engines.end()) {
        return std::reference_wrapper<Engine>(engines[ticker]);
    }
    else {
        return std::nullopt;
    }
}

void
Manager::printResults(manager::ClientManager& manager)
{
    for (const auto& client : manager.get_clients(true)) {
        float cap_remaining = client.capital_remaining;
        for (const auto& holding : client.holdings) {
            cap_remaining += holding.second * get_last_sell_price(holding.first);
        }
        std::cout << client.uid << " " << cap_remaining << std::endl;
    }
}

void
Manager::addInitialLiquidity(const std::string& ticker, float quantity, float price)
{
    MarketOrder to_add{"SIMULATED", messages::SIDE::SELL, "MARKET", ticker, quantity,
                       price};
    if (engines.find(ticker) != engines.end()) {
        engines[ticker].add_order_without_matching(to_add);
    }
}

void
Manager::addEngine(const std::string& ticker)
{
    if (engines.find(ticker) == engines.end()) {
        engines[ticker] = matching::Engine();
    }
}

float
Manager::get_last_sell_price(const std::string& ticker)
{
    if (engines.find(ticker) != engines.end()) {
        return engines[ticker].get_last_sell_price();
    }
    return 0;
}
} // namespace engine_manager
} // namespace nutc
