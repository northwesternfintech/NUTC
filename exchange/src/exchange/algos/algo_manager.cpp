#include "algo_manager.hpp"

#include "bot_mode/bot_mode.hpp"
#include "dev_mode/dev_mode.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/config/static/config.hpp"
#include "normal_mode/normal_mode.hpp"
#include "shared/types/decimal.hpp"

#include <memory>
#include <stdexcept>

namespace nutc::exchange {

using mode = shared::Mode;

void
AlgoInitializer::initialize_algo_management(TraderContainer& trader_container)
{
    shared::decimal_price start_cap = Config::get().constants().STARTING_CAPITAL;

    initialize_files();
    initialize_trader_container(trader_container, start_cap);
}

std::unique_ptr<AlgoInitializer>
AlgoInitializer::get_algo_initializer(mode mode)
{
    size_t wait_secs = Config::get().constants().WAIT_SECS;
    switch (mode) {
        case mode::dev:
            return std::make_unique<DevModeAlgoInitializer>(wait_secs, DEBUG_NUM_USERS);
        case mode::normal:
            return std::make_unique<NormalModeAlgoInitializer>(wait_secs);
        case mode::sandbox:
        case mode::bots_only:
            return std::make_unique<BotModeAlgoInitializer>();
    }
    throw std::runtime_error("Unknown exchange mode");
}

} // namespace nutc::exchange
