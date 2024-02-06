#include "ticker_state.hpp"

#include "exchange/bot_framework/bot_container.hpp"
#include "exchange/matching/manager/engine_manager.hpp"

namespace nutc {
namespace dashboard {

void
TickerState::on_tick(uint64_t)
{
    bots::BotContainer& bot_container = bots::BotContainerMapper::get_instance(TICKER);

    std::optional<engine_ref_t> engine =
        engine_manager::EngineManager::get_instance().get_engine(TICKER);
    assert(engine.has_value());

    nutc::matching::Engine& engine_ref = engine.value().get();
    midprice_ = engine_ref.get_midprice();
    spread_ = engine_ref.get_spread();
    auto [asks, bids] = engine_ref.get_spread_nums();
    num_asks_ = asks;
    num_bids_ = bids;

    theo_ = static_cast<float>(bot_container.get_theo());
    num_mm_bots_ = static_cast<uint>(bot_container.get_num_mm_bots());
}

} // namespace dashboard
} // namespace nutc
