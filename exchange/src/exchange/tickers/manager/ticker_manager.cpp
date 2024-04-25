#include "ticker_manager.hpp"

#include "exchange/bots/bot_container.hpp"
#include "exchange/config.h"
#include "exchange/logging.hpp"
#include "exchange/tickers/engine/level_update_generator.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/config/config_loader.hpp"

#include <iostream>
#include <iterator>

namespace nutc {
namespace engine_manager {

namespace {
using tick_update = messages::tick_update;

std::vector<std::string>
split_tick_updates(const tick_update& update)
{
    std::string buf = glz::write_json(update);
    if (false && buf.size() > MAX_PIPE_MSG_SIZE) {
        assert(update.matches.size() > 1 || update.ob_updates.size() > 1);
        auto mid_obs = update.ob_updates.begin() + (update.ob_updates.size() / 2);
        auto mid_matches = update.matches.begin() + (update.matches.size() / 2);
        tick_update tick_update_1{
            {update.ob_updates.begin(), mid_obs    },
            {update.matches.begin(),    mid_matches}
        };
        tick_update tick_update_2{
            {mid_obs,     update.ob_updates.end()},
            {mid_matches, update.matches.end()   }
        };
        auto res1 = split_tick_updates(tick_update_1);
        auto res2 = split_tick_updates(tick_update_2);
        std::ranges::move(res2, std::back_inserter(res1));
        return res1;
    }
    return {buf};
}
} // namespace

// TODO: helper functions
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
        std::vector<messages::match> glz_matches;
        glz_matches.reserve(matches_.size());
        for (const auto& match : matches_) {
            glz_matches.emplace_back(messages::match{
                match.ticker, match.side, match.price, match.quantity,
                match.buyer->get_id(), match.seller->get_id(),
                match.buyer->get_capital(), match.seller->get_capital()
            });
        }

        messages::tick_update updates{
            matching::get_updates(
                ticker, engine.last_order_container, engine.engine.get_order_container()
            ),
            glz_matches
        };
        log_i(
            main, "Broadcasting {} ob updates and {} matches for {}",
            updates.ob_updates.size(), updates.matches.size(), ticker
        );
        auto update_strs = split_tick_updates(updates);
        for (const auto& trader :
             traders::TraderContainer::get_instance().get_traders()) {
            trader.second->send_messages(update_strs);
        }
        engine.last_order_container = engine.engine.get_order_container();
        matches_.clear();
    }
}

double
EngineManager::get_midprice(const std::string& ticker)
{
    auto& engine = get_engine(ticker);
    double real_midprice = engine.engine.get_order_container().get_midprice();
    if (real_midprice == 0) {
        return engine.midprice;
    }
    engine.midprice = real_midprice;
    return real_midprice;
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
