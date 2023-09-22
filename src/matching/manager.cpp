#include "matching/manager.hpp"

namespace nutc {
namespace engine_manager {
std::optional<Engine>
Manager::getEngine(const std::string& ticker)
{
    if (engines.find(ticker) != engines.end()) {
        return engines[ticker];
    }
    else {
        return std::nullopt;
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
