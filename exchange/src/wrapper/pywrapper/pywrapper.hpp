#pragma once

#include "shared/types/decimal_price.hpp"
#include "shared/types/position.hpp"
#include "shared/types/ticker.hpp"
#include "shared/util.hpp"

namespace nutc::pywrapper {
using util::decimal_price;
using util::Side;
using util::Ticker;

using OrderBookUpdateFunction =
    std::function<void(const util::position& position_change)>;
using TradeUpdateFunction = std::function<void(const util::position& position_change)>;
using AccountUpdateFunction = std::function<
    void(const util::position& position_change, util::decimal_price held_capital)>;

OrderBookUpdateFunction ob_update_function();

TradeUpdateFunction trade_update_function();

AccountUpdateFunction account_update_function();

using LimitOrderFunction = std::function<bool(
    const std::string& side, const std::string& ticker, double quantity, double price,
    bool ioc
)>;
using MarketOrderFunction = std::function<
    bool(const std::string& side, const std::string& ticker, double quantity)>;

void create_api_module(
    LimitOrderFunction publish_limit_order, MarketOrderFunction publish_market_order
);

void run_initialization_code(const std::string& py_code);
} // namespace nutc::pywrapper
