#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

#include <glaze/glaze.hpp>

#include <cassert>

#include <memory>
#include <memory_resource>

namespace nutc::exchange {

class TraderIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = GenericTrader;
    using difference_type = std::ptrdiff_t;
    using pointer = std::shared_ptr<GenericTrader>*;
    using reference = GenericTrader&;

    TraderIterator(pointer ptr) : m_ptr(ptr) {}

    reference
    operator*() const
    {
        return *(*m_ptr);
    }

    pointer
    operator->()
    {
        return m_ptr;
    }

    TraderIterator&
    operator++()
    {
        m_ptr++;
        return *this;
    }

    TraderIterator
    operator++(int)
    {
        TraderIterator temp = *this;
        ++(*this);
        return temp;
    }

    bool
    operator==(const TraderIterator& other) const
    {
        return m_ptr == other.m_ptr;
    }

    bool
    operator!=(const TraderIterator& other) const
    {
        return m_ptr != other.m_ptr;
    }

private:
    pointer m_ptr;
};

static constexpr auto CACHED_TRADERS = 256;
static constexpr auto CACHED_TRADER_SIZE = CACHED_TRADERS * sizeof(GenericTrader);

class TraderContainer {
    std::array<std::byte, CACHED_TRADER_SIZE> buf_{};
    std::pmr::monotonic_buffer_resource res_{&buf_, sizeof(buf_)};
    std::pmr::polymorphic_allocator<GenericTrader> pmr_allocator_{&res_};
    std::vector<std::shared_ptr<GenericTrader>> traders_;

public:
    TraderContainer() = default;
    TraderContainer(const TraderContainer&) = delete;
    TraderContainer(TraderContainer&&) = delete;
    TraderContainer& operator=(const TraderContainer&) = delete;
    TraderContainer& operator=(TraderContainer&&) = delete;
    ~TraderContainer() = default;

    void
    add_trader(std::shared_ptr<GenericTrader> trader)
    {
        traders_.push_back(std::move(trader));
    }

    template <typename T, typename... Args>
    std::shared_ptr<T>
    add_trader(Args&&... args)
    {
        std::shared_ptr<GenericTrader> trader =
            make_shared_trader_<T>(std::forward<Args>(args)...);
        traders_.emplace_back(trader);
        return std::static_pointer_cast<T>(trader);
    }

    TraderIterator
    begin()
    {
        return traders_.data();
    }

    TraderIterator
    end()
    {
        return &traders_[traders_.size()];
    }

private:
    template <typename T, typename... Args>
    requires std::is_base_of_v<GenericTrader, T>
    std::shared_ptr<GenericTrader>
    make_shared_trader_(Args&&... args)
    {
        return std::allocate_shared<T>(pmr_allocator_, std::forward<Args>(args)...);
    }
};

} // namespace nutc::exchange
