#include "ticker_manager.hpp"

#include "exchange/bots/bot_container.hpp"

namespace nutc {
namespace engine_manager {

bool
EngineManager::has_engine(const std::string& ticker) const
{
    return engines_.find(ticker) != engines_.end();
}

NewEngine&
EngineManager::get_engine(const std::string& ticker)
{
    auto engine = engines_.find(ticker);
    assert(engine != engines_.end());
    return engine->second;
}

void
EngineManager::set_initial_price_(const std::string& ticker, double price)
{
    auto engine = engines_.find(ticker);
    assert(engine != engines_.end());
    // engine->second.set_initial_price(price);
}

void
EngineManager::add_engine(const std::string& ticker, double starting_price)
{
    engines_.emplace(ticker, NewEngine());
    set_initial_price_(ticker, starting_price);
    bot_containers_.emplace(ticker, bots::BotContainer(ticker, starting_price));
}

// for testing
void
EngineManager::add_engine(const std::string& ticker)
{
    engines_.emplace(ticker, NewEngine());
}

} // namespace engine_manager
} // namespace nutc
