#pragma once

#include "exchange/traders/trader_types/trader_interface.hpp"

#include <boost/unordered_map.hpp>
#include <glaze/glaze.hpp>

#include <cassert>

#include <memory>
#include <string>
#include <unordered_map>

namespace nutc {
namespace traders {
using lock_guard = std::lock_guard<std::mutex>;

// The traders themselves shouldn't need thread safety - maybe we should still consider
// adding, though
class TraderContainer {
    std::mutex trader_lock_{};
    boost::unordered_map<std::string, const std::shared_ptr<GenericTrader>> traders_{};

public:
    /**
     * @brief Add a trader to the trader manager, return a shared pointer to the trader
     */
    template <typename T, typename... Args>
    std::shared_ptr<T>
    add_trader(Args&&... args)
    {
        lock_guard lock{trader_lock_};

        std::shared_ptr<GenericTrader> trader =
            make_shared_trader_<T>(std::forward<Args>(args)...);
        traders_.insert({trader->get_id(), trader});
        return std::static_pointer_cast<T>(trader);
    }

    void
    remove_trader(const std::string& trader_id)
    {
        lock_guard lock{trader_lock_};
        if (traders_.find(trader_id) == traders_.end())
            return;
        traders_.erase(trader_id);
    }

    // This shouldn't need to be thread safe
    std::shared_ptr<GenericTrader>
    get_trader(const std::string& trader_id)
    {
        lock_guard lock{trader_lock_};
        assert(user_exists_(trader_id));
        return traders_.at(trader_id);
    }

    void
    broadcast_messages(const std::vector<std::string>& messages)
    {
        lock_guard lock{trader_lock_};
        for (const auto& trader_pair : traders_) {
            trader_pair.second->send_messages(messages);
        }
    }

    size_t
    num_traders()
    {
        lock_guard lock{trader_lock_};
        return traders_.size();
    }

    // TODO: REMOVE AFTER IMPROVING DASHBOARD
    const auto&
    get_traders()
    {
        lock_guard lock{trader_lock_};
        return traders_;
    }

    // Primarily for testing
    void
    reset()
    {
        lock_guard lock{trader_lock_};
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
