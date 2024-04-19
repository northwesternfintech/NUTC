#include "algo_manager.hpp"

#include "bot_mode/bot_mode.hpp"
#include "dev_mode/dev_mode.hpp"
#include "exchange/config.h"
#include "normal_mode/normal_mode.hpp"
#include "sandbox_mode/sandbox_mode.hpp"

#include <memory>
#include <optional>
#include <stdexcept>

namespace nutc {
namespace algos {

using mode = util::Mode;

std::unique_ptr<AlgoInitializer>
AlgoInitializer::get_algo_initializer(mode mode, std::optional<util::algorithm> sandbox)
{
    switch (mode) {
        case mode::dev:
            return std::make_unique<algos::DevModeAlgoInitializer>(DEBUG_NUM_USERS);
        case mode::normal:
            return std::make_unique<algos::NormalModeAlgoInitializer>();
        case mode::bots_only:
            return std::make_unique<algos::BotModeAlgoInitializer>();
        case mode::sandbox:
            auto& [uid, algo_id] = sandbox.value(); // NOLINT (unchecked-*)
            return std::make_unique<algos::SandboxAlgoInitializer>(uid, algo_id);
    }
    throw std::runtime_error("Unknown exchange mode");
}

} // namespace algos
} // namespace nutc
