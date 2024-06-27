#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

#include <boost/unordered_map.hpp>
#include <glaze/glaze.hpp>

#include <cassert>

#include <memory>
#include <memory_resource>

namespace nutc {
namespace traders {
using lock_guard = std::lock_guard<std::mutex>;

static constexpr auto CACHED_TRADERS = 256;
static constexpr auto CACHED_TRADER_SIZE = CACHED_TRADERS * sizeof(GenericTrader);

// The traders themselves shouldn't need thread safety - maybe we should still consider
// adding, though
class TraderContainer {
    std::array<std::byte, CACHED_TRADER_SIZE> buf;
    std::pmr::monotonic_buffer_resource res{&buf, sizeof(buf)};

    mutable std::mutex trader_lock_{};
    std::pmr::vector<std::shared_ptr<GenericTrader>> traders_{&res};

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
        traders_.emplace_back(trader);
        return std::static_pointer_cast<T>(trader);
    }

    void
    remove_trader(std::shared_ptr<traders::GenericTrader> to_remove)
    {
        lock_guard lock{trader_lock_};
        for (auto& trader : traders_) {
            if (trader == to_remove)
                trader->disable();
        }
    }

    size_t
    num_traders() const

    {
        lock_guard lock{trader_lock_};
        return traders_.size();
    }

    const std::pmr::vector<std::shared_ptr<GenericTrader>>&
    get_traders() const
    {
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

    TraderContainer() = default;
    ~TraderContainer() = default;

public:
    static TraderContainer&
    get_instance()
    {
        static TraderContainer instance{};
        return instance;
    }

    TraderContainer(const TraderContainer&) = delete;
    TraderContainer(TraderContainer&&) = delete;
    TraderContainer& operator=(const TraderContainer&) = delete;
    TraderContainer& operator=(TraderContainer&&) = delete;
};

} // namespace traders
} // namespace nutc
