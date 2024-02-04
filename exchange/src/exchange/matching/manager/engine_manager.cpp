#include "engine_manager.hpp"

namespace nutc {
namespace engine_manager {
std::optional<engine_ref_t>
Manager::get_engine(const std::string& ticker)
{
    auto engine = engines_.find(ticker);
    if (engine != engines_.end()) {
        return std::reference_wrapper<nutc::matching::Engine>(engine->second);
    }
    return std::nullopt;
}

void
Manager::add_initial_liquidity(const std::string& ticker, float quantity, float price)
{
    MarketOrder to_add{"SIMULATED", messages::SIDE::SELL, ticker, quantity, price};
    auto engine = engines_.find(ticker);
    if (engine != engines_.end()) {
        engine->second.add_order(to_add);
    }
}

void
Manager::add_engine(const std::string& ticker)
{
    if (engines_.find(ticker) == engines_.end()) {
        engines_.emplace(ticker, matching::Engine());
    }
}

} // namespace engine_manager
} // namespace nutc
