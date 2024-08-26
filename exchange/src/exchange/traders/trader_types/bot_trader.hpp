#pragma once

#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"
#include "shared/types/decimal.hpp"

#include <cassert>

#include <string>

namespace nutc::exchange {

class BotTrader : public GenericTrader {
    const shared::Ticker TICKER;
    const shared::decimal_price INTEREST_LIMIT;
    shared::decimal_price short_interest_{};
    shared::decimal_price long_interest_{};

    double open_bids_ = 0;
    double open_asks_ = 0;
    IncomingMessageQueue orders_;

public:
    double
    get_holdings() const
    {
        return GenericTrader::get_holdings(TICKER);
    }

    BotTrader(shared::Ticker ticker, shared::decimal_price interest_limit) :
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

    void
    disable() final
    {}

    [[nodiscard]] shared::decimal_price
    get_capital_utilization() const
    {
        shared::decimal_price capital_util =
            (get_long_interest() + get_short_interest()) / get_interest_limit();
        assert(capital_util <= 1.0);
        // assert(capital_util >= 0);
        return capital_util;
    }

    [[nodiscard]] shared::decimal_price
    get_long_interest() const
    {
        return long_interest_;
    }

    [[nodiscard]] shared::decimal_price
    get_interest_limit() const
    {
        return INTEREST_LIMIT;
    }

    [[nodiscard]] shared::decimal_price
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

    void notify_position_change(shared::position order) final;

    /**
     * midprice, theo
     */
    virtual void take_action(const shared_bot_state& shared_state) = 0;

    IncomingMessageQueue
    read_orders() override
    {
        IncomingMessageQueue ret{};
        orders_.swap(ret);
        return ret;
    }

protected:
    static double generate_gaussian_noise(double mean, double stddev);

    [[nodiscard]] shared::decimal_price
    compute_net_exposure_() const
    {
        return (get_long_interest() - get_short_interest());
    }

    void
    add_limit_order(
        shared::Side side, double quantity, shared::decimal_price price, bool ioc
    )
    {
        orders_.emplace_back(
            shared::timed_limit_order{TICKER, side, quantity, price, ioc}
        );
    }

    void
    add_market_order(shared::Side side, double quantity)
    {
        orders_.push_back(shared::timed_market_order{TICKER, side, quantity});
    }

    shared::decimal_price
    compute_capital_tolerance_()
    {
        return (shared::decimal_price{1.0} - get_capital_utilization())
               * (get_interest_limit() / 2.0);
    }

    void
    modify_short_capital(shared::decimal_price delta)
    {
        short_interest_ += delta;
    }

    void
    modify_long_capital(shared::decimal_price delta)
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

} // namespace nutc::exchange
