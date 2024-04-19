#include "test_mode.hpp"

#include "exchange/config.h"
#include "shared/config/config_loader.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "test_utils/helpers/test_trader.hpp"

#include <stdexcept>

namespace nutc {
namespace test_utils {

void
TestModeAlgoInitializer::initialize_trader_container(traders::TraderContainer& traders
) const
{
    int starting_cap = config::Config::get_instance().constants().STARTING_CAPITAL;

    for (const fs::path& filepath : algo_filepaths_)
        traders.add_trader<test_utils::TestTrader>(filepath, starting_cap);
}

} // namespace test_utils
} // namespace nutc
