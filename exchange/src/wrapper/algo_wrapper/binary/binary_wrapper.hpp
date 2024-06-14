
#pragma once

#include "wrapper/algo_wrapper/wrapper.hpp"

namespace nutc {
namespace wrapper {

class BinaryWrapper : public Wrapper {
public:
    BinaryWrapper(
        std::string algo, std::string trader_id, MarketOrderFunc publish_market_order
    );

    ~BinaryWrapper() override;

    void fire_on_trade_update(
        std::string ticker, std::string side, double price, double quantity
    ) const override;

    void fire_on_orderbook_update(
        std::string ticker, std::string side, double price, double quantity
    ) const override;

    void fire_on_account_update(
        std::string ticker, std::string side, double price, double quantity,
        double buyer_capital
    ) const override;

private:
    class StrategyObject;
    using MarketOrderFunc =
        std::function<bool(std::string, std::string, double, double)>;
    using DeleteStrategyFunc = void (*)(StrategyObject*);
    using OnTradeUpdateFunc =
        void (*)(StrategyObject*, std::string, std::string, double, double);
    using OnOrderBookUpdateFunc = OnTradeUpdateFunc;
    using OnAccountUpdateFunc =
        void (*)(StrategyObject*, std::string, std::string, double, double, double);

    StrategyObject* strategy_object_;

    DeleteStrategyFunc delete_strategy_;
    OnTradeUpdateFunc on_trade_update_;
    OnOrderBookUpdateFunc on_orderbook_update_;
    OnAccountUpdateFunc on_account_update_;

    void* dl_handle_;
    int memfd_;
};

} // namespace wrapper
} // namespace nutc
