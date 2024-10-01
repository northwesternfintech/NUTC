#pragma once

#include "wrapper/runtime/runtime.hpp"

namespace nutc::wrapper {

class CppRuntime : public Runtime {
public:
    CppRuntime(
        std::string algo, std::string trader_id, ExchangeCommunicator communicator
    );

    ~CppRuntime() override;

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
    using on_trade_update_func = void (*)(Strategy*, Ticker, Side, float, float);
    using on_orderbook_update_func = void (*)(Strategy*, Ticker, Side, float, float);
    using on_account_update_func =
        void (*)(Strategy*, Ticker, Side, float, float, float);

    on_trade_update_func on_trade_update_func_;
    on_orderbook_update_func on_orderbook_update_func_;
    on_account_update_func on_account_update_func_;

    Strategy* strategy_object_;
    void* dl_handle_;
    int fd_;
};
} // namespace nutc::wrapper
