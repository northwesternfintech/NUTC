#pragma once

#include "common/types/ticker.hpp"
#include "common/util.hpp"

#include <cstdint>

#include <functional>
#include <optional>
#include <string>

namespace nutc::linter {

using LimitOrderFunction = std::function<std::int64_t(
    common::Side side, common::Ticker ticker, float quantity, float price, bool ioc
)>;
using MarketOrderFunction =
    std::function<bool(common::Side side, common::Ticker ticker, float quantity)>;
using CancelOrderFunction =
    std::function<bool(common::Ticker ticker, std::int64_t order_id)>;
using PrintLnFunction = std::function<void(const std::string& text)>;

class Runtime {
public:
    virtual ~Runtime() = default;

    Runtime(const Runtime&) = default;
    Runtime(Runtime&&) noexcept = default;
    Runtime& operator=(const Runtime&) = default;
    Runtime& operator=(Runtime&&) noexcept = default;

    Runtime(
        std::string algo, LimitOrderFunction limit_order,
        MarketOrderFunction market_order, CancelOrderFunction cancel_order
    ) :
        algo_(std::move(algo)), m_limit_order_func(std::move(limit_order)),
        m_market_order_func(std::move(market_order)),
        m_cancel_order_func(std::move(cancel_order))
    {}

    virtual std::optional<std::string> init() = 0;
    virtual void fire_on_trade_update(
        common::Ticker ticker, common::Side side, float price, float quantity
    ) const = 0;
    virtual void fire_on_orderbook_update(
        common::Ticker ticker, common::Side side, float price, float quantity
    ) const = 0;
    virtual void fire_on_account_update(
        common::Ticker ticker, common::Side side, float price, float quantity,
        float buyer_capital
    ) const = 0;

protected:
    std::string algo_;
    LimitOrderFunction m_limit_order_func;
    MarketOrderFunction m_market_order_func;
    CancelOrderFunction m_cancel_order_func;

    static void
    log_text(const std::string&)
    {}
};

} // namespace nutc::linter
