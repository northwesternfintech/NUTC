#include "matching/engine_manager.hpp"

namespace nutc {
namespace engine_manager {
std::optional<EngineRef>
Manager::get_engine(const std::string& ticker)
{
    auto it = engines.find(ticker);
    if (it != engines.end()) {
        return std::reference_wrapper<Engine>(it->second);
    }
    return std::nullopt;
}

void
Manager::add_initial_liquidity(const std::string& ticker, float quantity, float price)
{
    MarketOrder to_add{"SIMULATED", messages::SIDE::SELL, "MARKET", ticker, quantity,
                       price};
    auto it = engines.find(ticker);
    if (it != engines.end()) {
        it->second.add_order_without_matching(to_add);
    }
}

void
Manager::add_engine(const std::string& ticker)
{
    if (engines.find(ticker) == engines.end()) {
        engines.emplace(ticker, matching::Engine());
    }
}

} // namespace engine_manager
} // namespace nutc
