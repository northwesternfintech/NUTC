#pragma once

#include "exchange/matching_cycle/dev/dev_strategy.hpp"
#include "exchange/sandbox_server/crow.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc::matching {

class SandboxMatchingCycle : public DevMatchingCycle {
public:
    SandboxMatchingCycle(
        TickerMapping tickers,
        std::vector<std::shared_ptr<traders::GenericTrader>>& traders,
        uint64_t expire_ticks
    ) : DevMatchingCycle(std::move(tickers), traders, expire_ticks)
    {}

private:
    virtual void
    before_cycle_(uint64_t) override
    {
        auto traders =
            sandbox::CrowServer::get_instance().get_and_clear_pending_traders();

        std::for_each(traders.begin(), traders.end(), [](auto&& trader) {
            traders::TraderContainer::get_instance().add_trader(trader);
        });
    }
};
} // namespace nutc::matching
