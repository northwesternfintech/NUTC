#pragma once

#include "exchange/matching_cycle/dev/dev_cycle.hpp"
#include "exchange/sandbox_server/crow.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc::exchange {

class SandboxMatchingCycle : public DevMatchingCycle {
    static constexpr std::uint16_t SANDBOX_SERVER_PORT = 18080;

    CrowServer http_server_{SANDBOX_SERVER_PORT};

public:
    SandboxMatchingCycle(
        TickerContainer tickers, TraderContainer& traders,
        common::decimal_price order_fee, common::decimal_quantity max_order_volume
    ) : DevMatchingCycle(std::move(tickers), traders, order_fee, max_order_volume)
    {}

private:
    void
    before_cycle_(uint64_t tick) override
    {
        auto traders = http_server_.get_and_clear_pending_traders();

        std::for_each(traders.begin(), traders.end(), [this](auto&& trader) {
            get_traders().add_trader(trader);
        });

        DevMatchingCycle::before_cycle_(tick);
    }
};
} // namespace nutc::exchange
