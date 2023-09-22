#pragma once
#include "matching/engine.hpp"

#include <optional>
#include <string>

using Engine = nutc::matching::Engine;

namespace nutc {
namespace engine_manager {
class Manager {
public:
    std::optional<Engine> getEngine(const std::string& ticker);
    void addEngine(const std::string& ticker);

private:
    std::map<std::string, matching::Engine> engines;
};
} // namespace engine_manager
} // namespace nutc
