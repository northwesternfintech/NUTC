#include "algo_manager.hpp"

#include "bot_mode/bot_mode.hpp"
#include "dev_mode/dev_mode.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/config/static/config.hpp"
#include "normal_mode/normal_mode.hpp"
#include "shared/types/decimal_price.hpp"

#include <memory>
#include <stdexcept>

namespace nutc {
namespace algos {

using mode = util::Mode;

void
AlgoInitializer::initialize_algo_management(traders::TraderContainer& trader_container)
{
    util::decimal_price start_cap = config::Config::get().constants().STARTING_CAPITAL;

    initialize_files();
    initialize_trader_container(trader_container, start_cap);
}

std::unique_ptr<AlgoInitializer>
AlgoInitializer::get_algo_initializer(mode mode)
{
    size_t wait_secs = config::Config::get().constants().WAIT_SECS;
    switch (mode) {
        case mode::dev:
            return std::make_unique<algos::DevModeAlgoInitializer>(
                wait_secs, DEBUG_NUM_USERS
            );
        case mode::normal:
            return std::make_unique<algos::NormalModeAlgoInitializer>(wait_secs);
        case mode::sandbox:
        case mode::bots_only:
            return std::make_unique<algos::BotModeAlgoInitializer>();
    }
    throw std::runtime_error("Unknown exchange mode");
}

} // namespace algos
} // namespace nutc
