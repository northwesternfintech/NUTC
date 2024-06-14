#pragma once

#include <functional>
#include <string>

namespace nutc {
namespace wrapper {

using MarketOrderFunc = std::function<bool(std::string, std::string, double, double)>;

class Wrapper {
public:
    Wrapper(std::string algo, std::string trader_id) :
        algo_(std::move(algo)), trader_id_(std::move(trader_id))
    {}

    virtual ~Wrapper() = default;

    virtual void fire_on_trade_update(
        std::string ticker, std::string side, double price, double quantity
    ) const = 0;
    virtual void fire_on_orderbook_update(
        std::string ticker, std::string side, double price, double quantity
    ) const = 0;
    virtual void fire_on_account_update(
        std::string ticker, std::string side, double price, double quantity,
        double buyer_capital
    ) const = 0;

    const std::string&
    get_trader_id() const
    {
        return trader_id_;
    }

protected:
    std::string algo_;
    std::string trader_id_;
};

} // namespace wrapper
} // namespace nutc
