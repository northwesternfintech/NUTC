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
Manager::add_initial_liquidity(const std::string& ticker, float quantity, float price)
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
} // namespace engine_manager
} // namespace nutc
