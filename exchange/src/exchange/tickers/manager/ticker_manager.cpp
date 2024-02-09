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
EngineManager::set_initial_price_(const std::string& ticker, float price)
{
    auto engine = engines_.find(ticker);
    assert(engine != engines_.end());
    engine->second.set_initial_price(price);
}

void
EngineManager::add_engine(const std::string& ticker, float starting_price)
{
    engines_.emplace(ticker, matching::Engine());
    set_initial_price_(ticker, starting_price);
    bot_containers_.emplace(
        ticker, bots::BotContainer(ticker, static_cast<double>(starting_price))
    );
}

void
EngineManager::add_engine(const std::string& ticker)
{
    engines_.emplace(ticker, matching::Engine());
}

} // namespace engine_manager
} // namespace nutc
