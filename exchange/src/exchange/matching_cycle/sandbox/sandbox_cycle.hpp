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

        dispatch_logs_();

        DevMatchingCycle::before_cycle_(tick);
    }

    void
    dispatch_logs_()
    {
        auto& traders = get_traders();

        std::for_each(traders.begin(), traders.end(), [this](GenericTrader& trader) {
            auto* algo_trader = dynamic_cast<AlgoTrader*>(&trader);
            if (!algo_trader
                || !http_server_.is_receiving_logs(algo_trader->get_id())) {
                return;
            }

            auto logs = algo_trader->read_logs();
            if (logs.empty()) {
                return;
            }

            // all this log stuff is pretty bad
            // one(of many) issue is that we are constantly acquiring and unacquiring a
            // lock here...
            for (const auto& log : logs) {
                http_server_.send_log(algo_trader->get_id(), log);
            }
        });
    }
};
} // namespace nutc::exchange
