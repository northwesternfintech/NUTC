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

using Mode = util::Mode;

std::unique_ptr<AlgoInitializer>
AlgoInitializer::get_algo_initializer(Mode mode, std::optional<util::algorithm> sandbox)
{
    switch (mode) {
        case Mode::DEV:
            return std::make_unique<algos::DevModeAlgoInitializer>(DEBUG_NUM_USERS);
        case Mode::NORMAL:
            return std::make_unique<algos::NormalModeAlgoInitializer>();
        case Mode::BOTS_ONLY:
            return std::make_unique<algos::BotModeAlgoInitializer>();
        case Mode::SANDBOX:
            auto& [uid, algo_id] = sandbox.value(); // NOLINT (unchecked-*)
            return std::make_unique<algos::SandboxAlgoInitializer>(uid, algo_id);
    }
    throw std::runtime_error("Unknown exchange mode");
}

} // namespace algos
} // namespace nutc
