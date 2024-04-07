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
namespace algo_mgmt {
using Mode = util::Mode;
using algorithm = util::algorithm;

std::unique_ptr<AlgoManager>
AlgoManager::get_algo_mgr(Mode mode, std::optional<algorithm> sandbox)
{
    switch (mode) {
        case Mode::DEV:
            return std::make_unique<algo_mgmt::DevModeAlgoManager>(DEBUG_NUM_USERS);
        case Mode::NORMAL:
            return std::make_unique<algo_mgmt::NormalModeAlgoManager>();
        case Mode::BOTS_ONLY:
            return std::make_unique<algo_mgmt::BotModeAlgoManager>();
        case Mode::SANDBOX:
            auto& [uid, algo_id] = sandbox.value(); // NOLINT (unchecked-*)
            return std::make_unique<algo_mgmt::SandboxAlgoManager>(uid, algo_id);
    }
    throw std::runtime_error("Unknown exchange mode");
}

} // namespace algo_mgmt
} // namespace nutc
