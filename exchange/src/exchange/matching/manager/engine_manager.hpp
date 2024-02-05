#pragma once

#include "exchange/config.h"
#include "exchange/matching/engine/engine.hpp"
#include "exchange/tick_manager/tick_observer.hpp"

#include <optional>
#include <string>

using engine_ref_t = std::reference_wrapper<nutc::matching::Engine>;

namespace nutc {
namespace engine_manager {

class EngineManager : public nutc::ticks::TickObserver {
public:
    std::optional<engine_ref_t> get_engine(const std::string& ticker);

    void add_engine(const std::string& ticker);

    void set_initial_price(const std::string& ticker, float price);

    // deprecated?
    void add_initial_liquidity(const std::string& ticker, float quantity, float price);

    void
    on_tick(uint64_t new_tick) override
    {
        if (new_tick < ORDER_EXPIRATION_TIME)
            return;
        for (auto& [ticker, engine] : engines_) {
            engine.remove_old_orders(new_tick, new_tick - ORDER_EXPIRATION_TIME);
        }
    }

private:
    std::map<std::string, matching::Engine> engines_;
    EngineManager() = default;

public:
    // fuck it, everything's a singleton

    static EngineManager&
    get_instance()
    {
        static EngineManager instance;
        return instance;
    }

    EngineManager(EngineManager const&) = delete;
    EngineManager operator=(EngineManager const&) = delete;
    EngineManager(EngineManager&&) = delete;
    EngineManager operator=(EngineManager&&) = delete;
};
} // namespace engine_manager
} // namespace nutc
