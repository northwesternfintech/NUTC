#pragma once
#include "exchange/exchange_state.hpp"
#include "exchange/matching_cycle/cycle_interface.hpp"
#include "exchange/orders/ticker_container.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc::exchange {

/**
 * @brief Barebones matching cycle. Likely to be overridden for more logging
 */
class BaseMatchingCycle : public MatchingCycleInterface {
    exchange_state& state_;
    common::decimal_price order_fee_;
    common::decimal_quantity max_cumulative_order_volume_;

public:
    // Require transfer of ownership
    BaseMatchingCycle(
        exchange_state& state, common::decimal_price order_fee,
        common::decimal_quantity max_order_volume
    ) :
        state_(state), order_fee_(order_fee),
        max_cumulative_order_volume_{max_order_volume}
    {}

protected:
    auto&
    get_tickers()
    {
        return state_.tickers;
    }

    TraderContainer&
    get_traders()
    {
        return state_.traders;
    }

    void before_cycle_(uint64_t) override;

    std::vector<OrderVariant> collect_orders(uint64_t) override;

    std::vector<tagged_match> match_orders_(std::vector<OrderVariant> orders) override;

    void handle_matches_(std::vector<tagged_match> matches) override;

    void
    post_cycle_(uint64_t) override
    {}

private:
    std::vector<common::position> get_orderbook_updates_();

    static std::vector<common::position>
    tagged_matches_to_positions(const std::vector<tagged_match>& matches);

    static void send_account_updates(const std::vector<tagged_match>& matches);
    void send_market_updates_(const std::vector<tagged_match>& matches);
};

} // namespace nutc::exchange
