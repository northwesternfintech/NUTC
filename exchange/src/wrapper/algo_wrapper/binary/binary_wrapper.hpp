#pragma once

#include "wrapper/algo_wrapper/wrapper.hpp"

namespace nutc::wrapper {

class BinaryWrapper : public Wrapper {
public:
    BinaryWrapper(
        std::string algo, std::string trader_id, ExchangeCommunicator communicator
    );

    ~BinaryWrapper() override;

private:
    void fire_on_trade_update(
        Ticker ticker, Side side, decimal_price price, decimal_quantity quantity
    ) const override;

    void fire_on_orderbook_update(
        Ticker ticker, Side side, decimal_price price, decimal_quantity quantity
    ) const override;

    void fire_on_account_update(
        Ticker ticker, Side side, decimal_price price, decimal_quantity quantity,
        decimal_price capital
    ) const override;

    using Strategy = void;
    using InitFunc = Strategy* (*)(MarketOrderFunction, LimitOrderFunction,
                                   CancelOrderFunction);
    using OnTradeUpdateFunc =
        void (*)(Strategy*, const std::string&, const std::string&, double, double);
    using OnOrderBookUpdateFunc = OnTradeUpdateFunc;
    using OnAccountUpdateFunc = void (*)(
        Strategy*, const std::string&, const std::string&, double, double, double
    );

    OnTradeUpdateFunc on_trade_update_func_;
    OnOrderBookUpdateFunc on_orderbook_update_func_;
    OnAccountUpdateFunc on_account_update_func_;

    Strategy* strategy_object_;
    void* dl_handle_;
    int fd_;
};
} // namespace nutc::wrapper
