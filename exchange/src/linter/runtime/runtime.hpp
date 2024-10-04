#pragma once

#include <cstdint>
#include <optional>

#include <functional>
#include <string>

namespace nutc::lint {

using LimitOrderFunction = std::function<std::int64_t(
    const std::string& side,
    const std::string& ticker,
    double quantity,
    double price,
    bool ioc
)>;
using MarketOrderFunction = std::function<
    bool(const std::string& side, const std::string& ticker, double quantity)>;
using CancelOrderFunction =
    std::function<bool(const std::string& ticker, std::int64_t order_id)>;

class Runtime {
public:
    virtual ~Runtime() = default;

    Runtime(const Runtime&) = default;
    Runtime(Runtime&&) noexcept = default;
    Runtime& operator=(const Runtime&) = default;
    Runtime& operator=(Runtime&&) noexcept = default;

    Runtime(
        std::string algo,
        LimitOrderFunction limit_order,
        MarketOrderFunction market_order,
        CancelOrderFunction cancel_order
    ) :
        algo_(std::move(algo)),
        m_limit_order_func(limit_order),
        m_market_order_func(market_order),
        m_cancel_order_func(cancel_order)
    {}

    virtual std::optional<std::string> init()  = 0;
    virtual void fire_on_trade_update(
        std::string ticker, std::string side, double price, double quantity
    ) const = 0;
    virtual void fire_on_orderbook_update(
        std::string ticker, std::string side, double price, double quantity
    ) const = 0;
    virtual void fire_on_account_update(
        std::string ticker,
        std::string side,
        double price,
        double quantity,
        double buyer_capital
    ) const = 0;

protected:
    std::string algo_;
    LimitOrderFunction m_limit_order_func;
    MarketOrderFunction m_market_order_func;
    CancelOrderFunction m_cancel_order_func;
};

} // namespace nutc::lint
