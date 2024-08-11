#pragma once

#include "exchange/traders/trader_container.hpp"
#include "shared/util.hpp"

namespace nutc {
namespace algos {

class AlgoInitializer {
    virtual void initialize_files() = 0;
    virtual void initialize_trader_container(
        traders::TraderContainer& manager, double start_cap
    ) const = 0;

public:
    void initialize_algo_management(traders::TraderContainer& trader_container);

    static std::unique_ptr<AlgoInitializer> get_algo_initializer(util::Mode mode);

    virtual ~AlgoInitializer() = default;
    AlgoInitializer() = default;
    AlgoInitializer(const AlgoInitializer&) = delete;
    AlgoInitializer(AlgoInitializer&&) = delete;
    AlgoInitializer& operator=(const AlgoInitializer&) = delete;
    AlgoInitializer& operator=(AlgoInitializer&&) = delete;
};

} // namespace algos
} // namespace nutc
