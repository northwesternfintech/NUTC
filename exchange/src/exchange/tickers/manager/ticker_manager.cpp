#include "ticker_manager.hpp"

#include "exchange/bots/bot_container.hpp"
#include "exchange/logging.hpp"
#include "exchange/rabbitmq/publisher/RabbitMQPublisher.hpp"
#include "exchange/tickers/engine/level_update_generator.hpp"

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
        glz_matches.reserve(matches_.size());
        for (const auto& match : matches_) {
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
EngineManager::get_engine_(const std::string& ticker)
{
    auto engine = engines_.find(ticker);
    assert(engine != engines_.end());
    return engine->second;
}

const Engine&
EngineManager::get_engine_(const std::string& ticker) const
{
    auto engine = engines_.find(ticker);
    assert(engine != engines_.end());
    return engine->second;
}

// TODO(anyone): make it clear this adds a bot container
void
EngineManager::add_engine(const std::string& ticker, double starting_price)
{
    engines_.emplace(ticker, Engine());
    bot_containers_.emplace(ticker, bots::BotContainer(ticker, starting_price));
}

// for testing
void
EngineManager::add_engine(const std::string& ticker)
{
    engines_.emplace(ticker, Engine());
}

} // namespace engine_manager
} // namespace nutc
