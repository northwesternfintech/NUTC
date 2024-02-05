#pragma once
#include "exchange/bot_framework/bots/mm.hpp"
#include "exchange/matching/manager/engine_manager.hpp"
#include "exchange/randomness/brownian.hpp"
#include "exchange/tick_manager/tick_observer.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

namespace nutc {

namespace bots {
using MarketOrder = messages::MarketOrder;
using Match = messages::Match;

class BotContainer : public ticks::TickObserver {
public:
    void
    on_tick(uint64_t) override
    {
        auto theo = theo_generator_.generate_next_price();
        auto current =
            engine_manager::EngineManager::get_instance().get_engine(ticker_);
        assert(current.has_value());
        auto current_price = current.value().get().get_midprice();
        auto orders =
            BotContainer::on_new_theo(static_cast<float>(theo), current_price);
    }

    void
    add_mm_bot(const std::string& bot_id, const MarketMakerBot& bot)
    {
        market_makers_[bot_id] = bot;
    }

    std::vector<MarketOrder>
    on_new_theo(float new_theo, float current)
    {
        std::vector<MarketOrder> orders;
        for (auto& [id, bot] : market_makers_) {
            std::vector<messages::MarketOrder> bot_orders =
                bot.take_action(new_theo, current);
            orders.insert(orders.end(), bot_orders.begin(), bot_orders.end());
        }
        return orders;
    }

    void
    process_bot_match(const Match& match)
    {
        auto match1 = market_makers_.find(match.buyer_id);
        auto match2 = market_makers_.find(match.seller_id);
        float total_cap = match.price * match.quantity;

        // Both have reduced their positions
        if (match1 != market_makers_.end()) {
            match1->second.modify_long_capital(-total_cap);
        }
        if (match2 != market_makers_.end()) {
            match2->second.modify_short_capital(-total_cap);
        }
    }

    BotContainer() = default;

    explicit BotContainer(std::string ticker, float starting_price) :
        ticker_(std::move(ticker)), theo_generator_(static_cast<double>(starting_price))
    {}

private:
    // TODO(stevenewald): make more elegant than string UUID
    std::unordered_map<std::string, MarketMakerBot> market_makers_;
    std::string ticker_;

    stochastic::BrownianMotion theo_generator_;
};
} // namespace bots
} // namespace nutc
