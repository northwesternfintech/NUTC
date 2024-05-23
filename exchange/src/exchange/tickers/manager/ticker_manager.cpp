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

void
EngineManager::on_tick(uint64_t new_tick)
{
    static auto& midprice_gauge = prometheus::BuildGauge()
                                      .Name("ticker_midprice")
                                      .Register(*metrics::Prometheus::get_registry());

    std::vector<messages::match> glz_matches = convert_to_glz(accum_matches_);

    for (auto& [ticker, engine] : engines_) {
        auto midprice = engine.orderbook.get_midprice();

        midprice_gauge
            .Add({
                {"ticker", ticker}
        })
            .Set(midprice);

        for (const auto& match : accum_matches_) {
            log_match(match);
        }

        engine.bot_container.generate_orders(midprice);
        engine.orderbook.expire_orders(new_tick - order_expiry_ticks);

        send_traders_updates(ticker, engine, glz_matches);

        engine.old_orderbook = engine.orderbook;
        accum_matches_.clear();
    }
}

std::vector<messages::match>
EngineManager::convert_to_glz(std::vector<matching::stored_match> matches)
{
    std::vector<messages::match> glz_matches{};
    glz_matches.reserve(matches.size());
    for (const auto& match : matches) {
        glz_matches.emplace_back(
            match.ticker, match.side, match.price, match.quantity,
            match.buyer->get_id(), match.seller->get_id(), match.buyer->get_capital(),
            match.seller->get_capital()
        );
    }
    return glz_matches;
}

void
EngineManager::send_traders_updates(
    const std::string& ticker, ticker_info& engine,
    std::vector<messages::match>& glz_matches
)
{
    messages::tick_update updates{
        matching::get_updates(ticker, engine.old_orderbook, engine.orderbook),
        glz_matches
    };
    std::string update_str = glz::write_json(updates);
    for (const auto& trader : traders::TraderContainer::get_instance().get_traders()) {
        trader->send_message(update_str);
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
