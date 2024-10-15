#pragma once

#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"

#include <cassert>

#include <string>

namespace nutc::exchange {

class BotTrader : public GenericTrader {
    const common::Ticker TICKER;
    IncomingMessageQueue orders_;

public:
    BotTrader(common::Ticker ticker, common::decimal_price interest_limit) :
        GenericTrader(generate_user_id(), interest_limit), TICKER(ticker)
    {}

    BotTrader(const BotTrader& other) = delete;
    BotTrader(BotTrader&& other) = default;
    BotTrader& operator=(const BotTrader& other) = delete;
    BotTrader& operator=(BotTrader&& other) = delete;

    common::Ticker
    get_ticker() const
    {
        return TICKER;
    }

    bool
    can_leverage() const final
    {
        return true;
    }

    void
    disable() final
    {}

    ~BotTrader() override = default;

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

    [[nodiscard]] common::decimal_price
    compute_net_exposure_() const
    {
        return (
            get_portfolio().get_long_interest() - get_portfolio().get_short_interest()
        );
    }

    [[nodiscard]] common::order_id_t
    add_limit_order(
        common::Side side, common::decimal_quantity quantity,
        common::decimal_price price, bool ioc
    )
    {
        common::limit_order order{TICKER,
                                  side,
                                  quantity,
                                  price,
                                  ioc,
                                  common::get_time(),
                                  common::generate_order_id()};
        orders_.emplace_back(order);
        return order.order_id;
    }

    void
    cancel_order(common::order_id_t order_id)
    {
        orders_.emplace_back(common::cancel_order{TICKER, order_id, common::get_time()}
        );
    }

    void
    add_market_order(common::Side side, common::decimal_quantity quantity)
    {
        orders_.emplace_back(
            common::market_order{TICKER, side, quantity, common::get_time()}
        );
    }

    void
    send_message(const std::string&) override
    {}

private:
    static std::string
    generate_user_id()
    {
        static uint64_t bot_id = 0;
        return "BOT_" + std::to_string(bot_id++);
    }
};

} // namespace nutc::exchange
