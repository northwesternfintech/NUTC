#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

#include <cassert>

#include <random>
#include <string>

namespace nutc {
namespace traders {

class BotTrader : public traders::GenericTrader {
    const util::Ticker TICKER;
    const double INTEREST_LIMIT;
    double short_interest_ = 0;
    double long_interest_ = 0;

    double open_bids_ = 0;
    double open_asks_ = 0;
    std::vector<limit_order> orders_{};

public:
    BotTrader(util::Ticker ticker, double interest_limit) :
        GenericTrader(generate_user_id(), interest_limit), TICKER(ticker),
        INTEREST_LIMIT(interest_limit)
    {}

    BotTrader(const BotTrader& other) = delete;
    BotTrader(BotTrader&& other) = default;
    BotTrader& operator=(const BotTrader& other) = delete;
    BotTrader& operator=(BotTrader&& other) = delete;

    // Bots should override if they shouldn't be able to leverage
    bool
    can_leverage() const override
    {
        return true;
    }

    [[nodiscard]] double
    get_capital_utilization() const
    {
        double capital_util =
            (get_long_interest() + get_short_interest()) / get_interest_limit();
        assert(capital_util <= 1);
        // assert(capital_util >= 0);
        return capital_util;
    }

    [[nodiscard]] double
    get_long_interest() const
    {
        return long_interest_;
    }

    [[nodiscard]] double
    get_interest_limit() const
    {
        return INTEREST_LIMIT;
    }

    [[nodiscard]] double
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

    void process_position_change(limit_order order) final;

    /**
     * midprice, theo
     */
    virtual void take_action(double, double) = 0;

    std::vector<limit_order>
    read_orders() override
    {
        auto ret = std::move(orders_);
        orders_.clear();
        return ret;
    }

protected:
    static double
    generate_gaussian_noise(double mean, double stddev)
    {
        static std::random_device rand{};
        static std::mt19937 gen{rand()};
        static std::normal_distribution<double> distr{mean, stddev};

        return distr(gen);
    }

    [[nodiscard]] double
    compute_net_exposure_() const
    {
        return (get_long_interest() - get_short_interest());
    }

    void
    add_order(util::Side side, double price, double quantity)
    {
        orders_.emplace_back(side, TICKER, price, quantity);
    }

    double
    compute_capital_tolerance_()
    {
        return (1 - get_capital_utilization()) * (get_interest_limit() / 3);
    }

    void
    modify_short_capital(double delta)
    {
        short_interest_ += delta;
    }

    void
    modify_long_capital(double delta)
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
    static uint64_t
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
