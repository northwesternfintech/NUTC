#pragma once
#include "exchange/bots/bot_container.hpp"
#include "exchange/orders/matching/engine.hpp"
#include "exchange/orders/storage/level_update_generator.hpp"
#include "exchange/orders/storage/orderbook.hpp"

namespace nutc {
namespace matching {

/**
 * @brief Contains the canonical reference to all data coupled to a ticker. Very useful
 * because we typically have to access all at once
 */
struct ticker_info {
    std::shared_ptr<matching::LevelUpdateGenerator> level_update_generator_;
    matching::OrderBook orderbook;

    Engine engine;
    bots::BotContainer bot_container;

    ticker_info(util::Ticker ticker, decimal_price order_fee) :
        ticker_info(ticker, 0.0, order_fee, {})
    {}

    // TODO: order fee should not be 0
    ticker_info(const config::ticker_config& config) :
        ticker_info(config.TICKER, config.STARTING_PRICE, 0.0, config.BOTS)
    {}

    ticker_info(
        util::Ticker ticker, double starting_price, decimal_price order_fee,
        std::vector<config::bot_config> config
    ) :
        level_update_generator_(std::make_shared<matching::LevelUpdateGenerator>()),
        orderbook(level_update_generator_), engine(order_fee),
        bot_container(ticker, starting_price, std::move(config))
    {}
};

} // namespace matching
} // namespace nutc
