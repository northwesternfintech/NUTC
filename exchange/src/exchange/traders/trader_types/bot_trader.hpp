#pragma once

#include "exchange/traders/trader_types/trader_interface.hpp"

#include <cassert>

#include <random>
#include <string>

namespace nutc {
namespace traders {

class BotTrader : public traders::GenericTrader {
    const std::string TICKER;
    const double INTEREST_LIMIT;
    double short_interest_ = 0;
    double long_interest_ = 0;

    double open_bids_ = 0;
    double open_asks_ = 0;
    std::vector<market_order> orders_{};

public:
    BotTrader(std::string ticker, double interest_limit) :
        GenericTrader(generate_user_id(), interest_limit), TICKER(std::move(ticker)),
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
        // assert(capital_util <= 1);
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

    void process_order_expiration(
        const std::string& ticker, util::Side side, double price, double quantity
    ) final;

    void process_order_match(
        const std::string& ticker, util::Side side, double price, double quantity
    ) final;

    /**
     * midprice, theo
     */
    virtual void take_action(double, double) = 0;

    std::vector<market_order>
    read_orders() override
    {
        auto ret = orders_;
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
    add_order(util::Side side, double quantity, double price)
    {
      price = std::max(price, 0.01);
        if (side == util::Side::buy) {
            modify_open_bids(quantity);
            modify_long_capital(quantity * price);
        }
        else {
            modify_open_asks(quantity);
            modify_short_capital(quantity * price);
        }
        orders_.emplace_back(side, TICKER, quantity, price);
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
    send_messages(const std::vector<std::string>&) override
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
