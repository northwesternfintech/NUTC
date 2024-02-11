#include "ticker_manager.hpp"

namespace nutc {
namespace engine_manager {
std::optional<engine_ref_t>
EngineManager::get_engine(const std::string& ticker)
{
    auto engine = engines_.find(ticker);
    if (engine != engines_.end()) {
        return std::reference_wrapper<nutc::matching::Engine>(engine->second);
    }
    return std::nullopt;
}

void
EngineManager::set_initial_price_(const std::string& ticker, double price)
{
    auto engine = engines_.find(ticker);
    assert(engine != engines_.end());
    engine->second.set_initial_price(price);
}

void
EngineManager::add_engine(const std::string& ticker, double starting_price)
{
    engines_.emplace(ticker, matching::Engine());
    set_initial_price_(ticker, starting_price);
    bot_containers_.emplace(
        std::piecewise_construct, std::forward_as_tuple(ticker),
        std::forward_as_tuple(ticker, starting_price)
    );
}

// for testing
void
EngineManager::add_engine(const std::string& ticker)
{
    engines_.emplace(ticker, matching::Engine());
}

} // namespace engine_manager
} // namespace nutc
