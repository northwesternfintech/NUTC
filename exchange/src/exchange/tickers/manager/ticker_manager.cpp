#include "ticker_manager.hpp"

#include "exchange/bots/bot_container.hpp"
#include "exchange/logging.hpp"
#include "exchange/rabbitmq/publisher/RabbitMQPublisher.hpp"
#include "exchange/tickers/engine/level_update_generator.hpp"
#include "shared/config/config_loader.hpp"
#include "shared/util.hpp"

namespace nutc {
namespace engine_manager {

void
EngineManager::on_tick(uint64_t new_tick)
{
    for (auto& [ticker, engine] : engines_) {
        std::vector<matching::StoredOrder> expired_orders =
            engine.expire_old_orders(new_tick);

        for (const auto& order : expired_orders) {
            order.trader->process_order_expiration(
                order.ticker, order.side, order.price, order.quantity
            );
        }

        // TODO: do this in a converter
        std::vector<Match> glz_matches;
        for (const auto& match : matches_) {
            match.buyer->process_order_match(
                match.ticker, messages::SIDE::BUY, match.price, match.quantity
            );
            match.seller->process_order_match(
                match.ticker, messages::SIDE::SELL, match.price, match.quantity
            );
            glz_matches.emplace_back(
                match.ticker, match.side, match.price, match.quantity,
                match.buyer->get_id(), match.seller->get_id(),
                match.buyer->get_capital(), match.seller->get_capital()
            );
        }

        std::vector<ObUpdate> updates = matching::get_updates(
            ticker, last_order_containers_[ticker], engine.get_order_container()
        );
        last_order_containers_[ticker] = engine.get_order_container();

        log_i(main, "Broadcasting {} updates for {}", updates.size(), ticker);

        rabbitmq::RabbitMQPublisher::broadcast_ob_updates(updates);

        rabbitmq::RabbitMQPublisher::broadcast_matches(glz_matches);
        matches_.clear();
    }
}

bool
EngineManager::has_engine(const std::string& ticker) const
{
    return engines_.find(ticker) != engines_.end();
}

Engine&
EngineManager::get_engine(const std::string& ticker)
{
    auto engine = engines_.find(ticker);
    assert(engine != engines_.end());
    return engine->second;
}

// TODO(stevenewald): consolidate this so no longer necessary
void
EngineManager::add_engine(const std::string& ticker, double starting_price)
{
    size_t exp_ticks =
        config::Config::get_instance().constants().ORDER_EXPIRATION_TICKS;
    engines_.emplace(ticker, exp_ticks);
    bot_containers_.emplace(ticker, bots::BotContainer(ticker, starting_price));
}

void
EngineManager::add_engine(const std::string& ticker)
{
    add_engine(ticker, 0);
}

} // namespace engine_manager
} // namespace nutc
