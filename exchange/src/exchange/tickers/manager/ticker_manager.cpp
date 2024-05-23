#include "ticker_manager.hpp"

#include "exchange/metrics/prometheus.hpp"
#include "exchange/tickers/engine/level_update_generator.hpp"
#include "exchange/traders/trader_container.hpp"

#include <prometheus/counter.h>
#include <prometheus/gauge.h>

namespace nutc {
namespace engine_manager {

using tick_update = messages::tick_update;

// NOTE: we can still run into buffer errors if we have an extremely large message
// buffer to send and we split it up into very small chunks This only happens in the
// extreme extreme case (say, 100k orders with 8k max msg size)

void
log_match(const matching::stored_match& order)
{
    static auto& match_counter = prometheus::BuildCounter()
                                     .Name("matched_orders")
                                     .Register(*metrics::Prometheus::get_registry());
    match_counter
        .Add({
            {"side",        "SELL"                  },
            {"ticker",      order.ticker            },
            {"trader_type", order.seller->get_type()},
            {"traderid",    order.seller->get_id()  },
    })
        .Increment(order.quantity);

    match_counter
        .Add({
            {"side",        "BUY"                  },
            {"ticker",      order.ticker           },
            {"trader_type", order.buyer->get_type()},
            {"traderid",    order.buyer->get_id()  },
    })
        .Increment(order.quantity);
}

size_t
EngineManager::match_order(const matching::stored_order& order)
{
    auto& ticker = get_engine(order.ticker);
    std::vector<matching::stored_match> matches =
        ticker.engine.match_order(ticker.orderbook, order);
    std::ranges::move(matches, std::back_inserter(accum_matches_));
    return matches.size();
}

// TODO: helper functions/cleanup
void
EngineManager::on_tick(uint64_t new_tick)
{
    static auto& midprice_gauge = prometheus::BuildGauge()
                                      .Name("ticker_midprice")
                                      .Register(*metrics::Prometheus::get_registry());

    for (auto& [ticker, engine] : engines_) {
        auto midprice = engine.orderbook.get_midprice();
        midprice_gauge
            .Add({
                {"ticker", ticker}
        })
            .Set(midprice);
        engine.bot_container.generate_orders(midprice);
    }

    for (auto& [ticker, engine] : engines_) {
        engine.orderbook.expire_orders(new_tick - order_expiry_ticks);

        // TODO: do this in a converter
        std::vector<messages::match> glz_matches{};
        glz_matches.reserve(accum_matches_.size());
        for (const auto& match : accum_matches_) {
            log_match(match);
            glz_matches.emplace_back(
                match.ticker, match.side, match.price, match.quantity,
                match.buyer->get_id(), match.seller->get_id(),
                match.buyer->get_capital(), match.seller->get_capital()
            );
        }
        auto ob_updates =
            engine.level_update_generator_->get_updates(ticker, engine.orderbook);
        messages::tick_update updates{ob_updates, glz_matches};
        std::string update_str = glz::write_json(updates);
        for (const auto& trader :
             traders::TraderContainer::get_instance().get_traders()) {
            trader->send_message(update_str);
        }
        accum_matches_.clear();
        engine.level_update_generator_->reset();
    }
}

double
EngineManager::get_midprice(const std::string& ticker) const
{
    return get_engine(ticker).orderbook.get_midprice();
}

bool
EngineManager::has_engine(const std::string& ticker) const
{
    return engines_.find(ticker) != engines_.end();
}

ticker_info&
EngineManager::get_engine(const std::string& ticker)
{
    assert(has_engine(ticker));
    auto engine = engines_.find(ticker);
    return engine->second;
}

const ticker_info&
EngineManager::get_engine(const std::string& ticker) const
{
    assert(has_engine(ticker));
    auto engine = engines_.find(ticker);
    return engine->second;
}

void
EngineManager::add_engine(const config::ticker_config& config)
{
    engines_.emplace(
        config.TICKER,
        ticker_info(config.TICKER, config.STARTING_PRICE, order_fee, config.BOTS)
    );
}

void
EngineManager::add_engine(const std::string& ticker)
{
    add_engine(config::ticker_config{ticker, 0, {}});
}

} // namespace engine_manager
} // namespace nutc
