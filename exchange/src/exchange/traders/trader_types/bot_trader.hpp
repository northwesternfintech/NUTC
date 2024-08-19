#pragma once

#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"
#include "shared/types/decimal_price.hpp"

#include <cassert>

#include <string>

namespace nutc {
namespace traders {

class BotTrader : public traders::GenericTrader {
    const util::Ticker TICKER;
    const util::decimal_price INTEREST_LIMIT;
    util::decimal_price short_interest_{};
    util::decimal_price long_interest_{};

    double open_bids_ = 0;
    double open_asks_ = 0;
    std::vector<messages::limit_order> orders_;

public:
    double
    get_holdings() const
    {
        return GenericTrader::get_holdings(TICKER);
    }

    BotTrader(util::Ticker ticker, util::decimal_price interest_limit) :
        GenericTrader(generate_user_id(), interest_limit), TICKER(ticker),
        INTEREST_LIMIT(interest_limit)
    {}

    BotTrader(const BotTrader& other) = delete;
    BotTrader(BotTrader&& other) = default;
    BotTrader& operator=(const BotTrader& other) = delete;
    BotTrader& operator=(BotTrader&& other) = delete;

    bool
    can_leverage() const final
    {
        return true;
    }

    [[nodiscard]] util::decimal_price
    get_capital_utilization() const
    {
        util::decimal_price capital_util =
            (get_long_interest() + get_short_interest()) / get_interest_limit();
        assert(capital_util <= 1.0);
        // assert(capital_util >= 0);
        return capital_util;
    }

    [[nodiscard]] util::decimal_price
    get_long_interest() const
    {
        return long_interest_;
    }

    [[nodiscard]] util::decimal_price
    get_interest_limit() const
    {
        return INTEREST_LIMIT;
    }

    [[nodiscard]] util::decimal_price
    get_short_interest() const
    {
        return short_interest_;
    }

    [[nodiscard]] double
    get_open_bids() const
    {
        return open_bids_;
    }

    [[nodiscard]] double
    get_open_asks() const
    {
        return open_asks_;
    }

    ~BotTrader() override = default;

    void process_position_change(util::position order) final;

    /**
     * midprice, theo
     */
    virtual void take_action(const bots::shared_bot_state& shared_state) = 0;

    std::vector<messages::limit_order>
    read_orders() override
    {
        auto ret = std::move(orders_);
        orders_.clear();
        return ret;
    }

protected:
    [[nodiscard]] util::decimal_price
    compute_net_exposure_() const
    {
        return (get_long_interest() - get_short_interest());
    }

    void
    add_limit_order(
        util::Side side, double quantity, util::decimal_price price, bool ioc
    )
    {
        orders_.emplace_back(side, TICKER, quantity, price, ioc);
    }

    void
    add_market_order(util::Side side, double quantity)
    {
        orders_.emplace_back(side, TICKER, quantity);
    }

    util::decimal_price
    compute_capital_tolerance_()
    {
        return (util::decimal_price{1.0} - get_capital_utilization())
               * (get_interest_limit() / 2.0);
    }

    void
    modify_short_capital(util::decimal_price delta)
    {
        short_interest_ += delta;
    }

    void
    modify_long_capital(util::decimal_price delta)
    {
        long_interest_ += delta;
    }

    /**
     * @brief Called by the bot(derived class) when a bid position is opened
     */
    void
    modify_open_bids(double delta)
    {
        open_bids_ += delta;
    }

    /**
     * @brief Called by the bot (derived class) when an ask position is opened
     */
    void
    modify_open_asks(double delta)
    {
        open_asks_ += delta;
    }

    void
    send_message(const std::string&) override
    {}

private:
    static inline uint64_t
    get_and_increment_user_id()
    {
        static uint64_t user_id = 0;
        return user_id++;
    }

    static std::string
    generate_user_id()
    {
        return "BOT_" + std::to_string(get_and_increment_user_id());
    }
};

} // namespace traders
} // namespace nutc
