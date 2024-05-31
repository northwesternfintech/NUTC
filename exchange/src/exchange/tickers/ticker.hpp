#pragma once
#include "exchange/bots/bot_container.hpp"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/tickers/engine/level_update_generator.hpp"
#include "exchange/tickers/engine/orderbook.hpp"

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

    ticker_info(std::string ticker, double order_fee) :
        ticker_info(std::move(ticker), 0, order_fee, {})
    {}

    // TODO: order fee should not be 0
    ticker_info(const config::ticker_config& config) :
        ticker_info(config.TICKER, config.STARTING_PRICE, 0, config.BOTS)
    {}

    ticker_info(
        std::string ticker, double starting_price, double order_fee,
        std::vector<config::bot_config> config
    ) :
        level_update_generator_(std::make_shared<matching::LevelUpdateGenerator>()),
        orderbook(level_update_generator_), engine(order_fee),
        bot_container(std::move(ticker), starting_price, std::move(config))
    {}
};

} // namespace matching
} // namespace nutc
