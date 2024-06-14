#pragma once

#include "wrapper/algo_wrapper/wrapper.hpp"

namespace nutc {
namespace wrapper {

class PyWrapper : public Wrapper {
public:
    PyWrapper(
        std::string algo, std::string trader_id, MarketOrderFunc publish_market_order
    );

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
};

} // namespace wrapper
} // namespace nutc
