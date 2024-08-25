#pragma once

#include "exchange/matching_cycle/dev/dev_cycle.hpp"
#include "exchange/sandbox_server/crow.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc::exchange {

class SandboxMatchingCycle : public DevMatchingCycle {
public:
    SandboxMatchingCycle(TickerMapping tickers, TraderContainer& traders) :
        DevMatchingCycle(std::move(tickers), traders)
    {}

private:
    virtual void
    before_cycle_(uint64_t) override
    {
        auto traders = CrowServer::get_instance().get_and_clear_pending_traders();

        std::for_each(traders.begin(), traders.end(), [this](auto&& trader) {
            get_traders().add_trader(trader);
        });
    }
};
} // namespace nutc::exchange
