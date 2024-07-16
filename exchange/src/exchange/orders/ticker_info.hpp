#pragma once
#include "exchange/bots/bot_container.hpp"
#include "exchange/matching/engine.hpp"
#include "exchange/orders/orderbook/cancellable_orderbook.hpp"
#include "exchange/orders/orderbook/level_tracked_orderbook.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"

#include <absl/hash/hash.h>

namespace nutc {
namespace matching {

template <typename BaseOrderBookT>
using DecoratedOrderBook = LevelTrackedOrderbook<CancellableOrderBook<BaseOrderBookT>>;

/**
 * @brief Contains the canonical reference to all data coupled to a ticker. Very useful
 * because we typically have to access all at once
 */
struct ticker_info {
    DecoratedOrderBook<LimitOrderBook> orderbook;

    Engine engine;
    bots::BotContainer bot_container;

    ticker_info(util::Ticker ticker, util::decimal_price order_fee) :
        ticker_info(ticker, 0.0, order_fee, {})
    {}

    // TODO: order fee should not be 0
    ticker_info(const config::ticker_config& config) :
        ticker_info(config.TICKER, config.STARTING_PRICE, 0.0, config.BOTS)
    {}

    ticker_info(
        util::Ticker ticker, double starting_price, util::decimal_price order_fee,
        std::vector<config::bot_config> config
    ) :
        orderbook(), engine(order_fee),
        bot_container(ticker, starting_price, std::move(config))
    {}
};

using TickerMapping =
    emhash7::HashMap<util::Ticker, ticker_info, absl::Hash<util::Ticker>>;

} // namespace matching
} // namespace nutc
