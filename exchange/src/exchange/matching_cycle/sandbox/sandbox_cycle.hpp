#pragma once

#include "exchange/matching_cycle/dev/dev_cycle.hpp"
#include "exchange/sandbox_server/crow.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc::matching {

class SandboxMatchingCycle : public DevMatchingCycle {
public:
    SandboxMatchingCycle(
        TickerMapping tickers, traders::TraderContainer& traders, uint64_t expire_ticks
    ) : DevMatchingCycle(std::move(tickers), traders, expire_ticks)
    {}

private:
    virtual void
    before_cycle_(uint64_t) override
    {
        auto traders =
            sandbox::CrowServer::get_instance().get_and_clear_pending_traders();

        std::for_each(traders.begin(), traders.end(), [this](auto&& trader) {
            get_trader_container().add_trader(trader);
        });
    }
};
} // namespace nutc::matching
