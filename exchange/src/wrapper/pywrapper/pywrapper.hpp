#pragma once

#include "common/types/decimal.hpp"
#include "common/types/position.hpp"
#include "common/util.hpp"

namespace nutc::wrapper {

using OrderBookUpdateFunction =
    std::function<void(const common::position& position_change)>;
using TradeUpdateFunction =
    std::function<void(const common::position& position_change)>;
using AccountUpdateFunction = std::function<
    void(const common::position& position_change, common::decimal_price held_capital)>;

OrderBookUpdateFunction ob_update_function();

TradeUpdateFunction trade_update_function();

AccountUpdateFunction account_update_function();

using LimitOrderFunction = std::function<common::order_id_t(
    const std::string& side, const std::string& ticker, double quantity, double price,
    bool ioc
)>;
using MarketOrderFunction = std::function<
    bool(const std::string& side, const std::string& ticker, double quantity)>;
using CancelOrderFunction = std::function<void(common::order_id_t order_id)>;

void create_api_module(
    LimitOrderFunction publish_limit_order, MarketOrderFunction publish_market_order,
    CancelOrderFunction cancel_order
);

void run_initialization_code(const std::string& py_code);
} // namespace nutc::wrapper
