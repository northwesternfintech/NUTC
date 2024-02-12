#include "ticker_manager.hpp"

namespace nutc {
namespace engine_manager {

void
EngineManager::set_initial_price_(const std::string& ticker, float price)
{
    MarketOrder to_add1{
        "SIMULATED", messages::SIDE::SELL, ticker, 1, static_cast<float>(price * 1.01)
    }; // NOLINT(*)
    MarketOrder to_add2{
        "SIMULATED", messages::SIDE::BUY, ticker, 1, static_cast<float>(price * .99)
    }; // NOLINT(*)
    auto engine = engines_.find(ticker);
    assert(engine != engines_.end());
    engine->second.add_order(to_add1);
    engine->second.add_order(to_add2);
}

void
EngineManager::add_initial_liquidity(
    const std::string& ticker, float quantity, float price
)
{
    MarketOrder to_add1{
        "SIMULATED", messages::SIDE::SELL, ticker, quantity, price
    }; // NOLINT(*)
    auto engine = engines_.find(ticker);
    assert(engine != engines_.end());
    engine->second.add_order(to_add1);
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
