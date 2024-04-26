#pragma once

#include "exchange/traders/trader_types/trader_interface.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <glaze/glaze.hpp>

#include <cassert>

#include <memory>
#include <string>
#include <unordered_map>

namespace nutc {
namespace traders {

class TraderContainer {
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
        return std::static_pointer_cast<T>(trader);
    }

    void
    remove_trader(const std::string& trader_id)
    {
        assert(traders_.find(trader_id) != traders_.end());
        traders_.erase(trader_id);
    }

    [[nodiscard]] std::shared_ptr<GenericTrader>
    get_trader(const std::string& trader_id) const
    {
        assert(user_exists_(trader_id));
        return traders_.at(trader_id);
    }

    void
    broadcast_messages(const std::vector<std::string>& messages)
    {
        for (const auto& trader_pair : traders_) {
            trader_pair.second->send_messages(messages);
        }
    }

    size_t
    num_traders() const
    {
        return traders_.size();
    }

    // TODO: REMOVE AFTER IMPROVING DASHBOARD
    const auto&
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

    TraderContainer() = default;
    ~TraderContainer() = default;

public:
    // Singleton
    static TraderContainer&
    get_instance()
    {
        static TraderContainer instance;
        return instance;
    }

    TraderContainer(const TraderContainer&) = delete;
    TraderContainer(TraderContainer&&) = delete;
    TraderContainer& operator=(const TraderContainer&) = delete;
    TraderContainer& operator=(TraderContainer&&) = delete;
};

} // namespace traders
} // namespace nutc
