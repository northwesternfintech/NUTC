#pragma once

#include "linter/runtime/runtime.hpp"

namespace nutc::lint {

class CppRuntime : public Runtime {
public:
    CppRuntime(
        std::string algo, LimitOrderFunction limit_order,
        MarketOrderFunction market_order, CancelOrderFunction cancel_order
    );

    ~CppRuntime() override;

    std::optional<std::string> init() override;

    void fire_on_trade_update(
        common::Ticker ticker, common::Side side, double price, double quantity
    ) const override;

    void fire_on_orderbook_update(
        common::Ticker ticker, common::Side side, double price, double quantity
    ) const override;

    void fire_on_account_update(
        common::Ticker ticker, common::Side side, double price, double quantity,
        double capital
    ) const override;

private:
    using Strategy = void;
    using InitFunc = Strategy* (*)(MarketOrderFunction, LimitOrderFunction,
                                   CancelOrderFunction);
    using OnTradeUpdateFunc =
        void (*)(Strategy*, common::Ticker, common::Side, double, double);
    using OnOrderBookUpdateFunc = OnTradeUpdateFunc;
    using OnAccountUpdateFunc =
        void (*)(Strategy*, common::Ticker, common::Side, double, double, double);

    OnTradeUpdateFunc on_trade_update_func_;
    OnOrderBookUpdateFunc on_orderbook_update_func_;
    OnAccountUpdateFunc on_account_update_func_;

    Strategy* strategy_object_;
    void* dl_handle_;
    int fd_;
};
} // namespace nutc::lint
