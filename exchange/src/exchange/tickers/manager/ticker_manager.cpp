#include "ticker_manager.hpp"

#include "exchange/config.h"
#include "exchange/metrics/prometheus.hpp"
#include "exchange/tickers/engine/level_update_generator.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/config/config_loader.hpp"

#include <prometheus/counter.h>

#include <algorithm>
#include <iterator>

namespace nutc {
namespace engine_manager {

using tick_update = messages::tick_update;

// NOTE: we can still run into buffer errors if we have an extremely large message
// buffer to send and we split it up into very small chunks This only happens in the
// extreme extreme case (say, 100k orders with 8k max msg size)
std::vector<std::string>
EngineManager::split_tick_updates_(const tick_update& update)
{
    std::string buf = glz::write_json(update);
    if (buf.size() > MAX_PIPE_MSG_SIZE) {
        assert(update.matches.size() > 1 || update.ob_updates.size() > 1);
        auto mid_obs =
            update.ob_updates.begin()
            + static_cast<std::ptrdiff_t>((update.ob_updates.size() + 1) / 2);
        auto mid_matches =
            update.matches.begin()
            + static_cast<std::ptrdiff_t>((update.matches.size() + 1) / 2);
        tick_update tick_update_1{
            {update.ob_updates.begin(), mid_obs    },
            {update.matches.begin(),    mid_matches}
        };
        tick_update tick_update_2{
            {mid_obs,     update.ob_updates.end()},
            {mid_matches, update.matches.end()   }
        };
        auto res1 = split_tick_updates_(tick_update_1);
        auto res2 = split_tick_updates_(tick_update_2);
        std::ranges::copy(res2, std::back_inserter(res1));
        return res1;
    }
    return {buf};
}

void
log_match(const matching::stored_match& order)
{
    static auto& match_counter = prometheus::BuildCounter()
                                     .Name("matched_orders")
                                     .Register(*metrics::Prometheus::get_registry());
    if (order.seller->record_metrics()) {
        match_counter
            .Add({
                {"traderid", order.seller->get_id()        },
                {"ticker",   order.ticker                  },
                {"side",     "SELL"                        },
                {"quantity", std::to_string(order.quantity)}
        })
            .Increment(order.quantity);
    }

    if (order.buyer->record_metrics()) {
        match_counter
            .Add({
                {"traderid", order.buyer->get_id()         },
                {"ticker",   order.ticker                  },
                {"side",     "BUY"                         },
                {"quantity", std::to_string(order.quantity)}
        })
            .Increment(order.quantity);
    }
}

// TODO: helper functions/cleanup
void
EngineManager::on_tick(uint64_t new_tick)
{
    for (auto& [ticker, engine] : engines_) {
        std::vector<matching::stored_order> expired_orders =
            engine.engine.expire_old_orders(new_tick);

        for (const auto& order : expired_orders) {
            order.trader->process_order_expiration(
                order.ticker, order.side, order.price, order.quantity
            );
        }

        // TODO: do this in a converter
        std::vector<messages::match> glz_matches{};
        glz_matches.reserve(matches_.size());
        for (const auto& match : matches_) {
            log_match(match);
            glz_matches.emplace_back(
                match.ticker, match.side, match.price, match.quantity,
                match.buyer->get_id(), match.seller->get_id(),
                match.buyer->get_capital(), match.seller->get_capital()
            );
        }

        messages::tick_update updates{
            matching::get_updates(
                ticker, engine.last_order_container, engine.engine.get_order_container()
            ),
            glz_matches
        };
        auto update_strs = split_tick_updates_(updates);
        for (const auto& trader :
             traders::TraderContainer::get_instance().get_traders()) {
            trader->send_messages(update_strs);
        }
        engine.last_order_container = engine.engine.get_order_container();
        matches_.clear();
    }
}

double
EngineManager::get_midprice(const std::string& ticker) const
{
    return get_engine(ticker).engine.get_order_container().get_midprice();
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

// TODO(stevenewald): consolidate this so no longer necessary
void
EngineManager::add_engine(const std::string& ticker, double starting_price)
{
    size_t exp_ticks = config::Config::get().constants().ORDER_EXPIRATION_TICKS;
    engines_.emplace(ticker, ticker_info(ticker, exp_ticks, starting_price));
}

void
EngineManager::add_engine(const std::string& ticker)
{
    add_engine(ticker, 0);
}

} // namespace engine_manager
} // namespace nutc
