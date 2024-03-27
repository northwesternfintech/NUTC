#pragma once
#include "exchange/traders/trader_types/generic_trader.hpp"

#include <glaze/glaze.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace nutc {
namespace manager {

class TraderManager {
    std::unordered_map<std::string, const std::shared_ptr<GenericTrader>> traders_;

public:
    /**
     * @brief Add a trader to the trader manager, return a shared pointer to the trader
     */
    template <typename T, typename... Args>
    std::shared_ptr<T>
    add_trader(Args&&... args)
    {
        std::shared_ptr<GenericTrader> trader =
            make_shared_trader_<T>(std::forward<Args>(args)...);
        traders_.insert({trader->get_id(), trader});
        return std::dynamic_pointer_cast<T>(trader);
    }

    [[nodiscard]] std::shared_ptr<GenericTrader>
    get_trader(const std::string& trader_id) const
    {
        assert(user_exists_(trader_id));
        return traders_.at(trader_id);
    }

    std::unordered_map<std::string, const std::shared_ptr<GenericTrader>>&
    get_traders()
    {
        return traders_;
    }

    const std::unordered_map<std::string, const std::shared_ptr<GenericTrader>>&
    get_traders() const
    {
        return traders_;
    }

    // Primarily for testing
    void
    reset()
    {
        traders_.clear();
    }

private:
    template <typename T, typename... Args>
    requires std::is_base_of_v<GenericTrader, T>
    std::shared_ptr<GenericTrader>
    make_shared_trader_(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    bool
    user_exists_(const std::string& user_id) const
    {
        return traders_.contains(user_id);
    }

    TraderManager() = default;
    ~TraderManager() = default;

public:
    // Singleton
    static TraderManager&
    get_instance()
    {
        static TraderManager instance;
        return instance;
    }

    TraderManager(const TraderManager&) = delete;
    TraderManager(TraderManager&&) = delete;
    TraderManager& operator=(const TraderManager&) = delete;
    TraderManager& operator=(TraderManager&&) = delete;
};

} // namespace manager
} // namespace nutc
