#pragma once

#include "exchange/traders/trader_container.hpp"
#include "shared/types/decimal.hpp"
#include "shared/util.hpp"

namespace nutc::exchange {

class AlgoInitializer {
    virtual void initialize_files() = 0;
    virtual void initialize_trader_container(
        TraderContainer& manager, shared::decimal_price start_cap
    ) const = 0;

public:
    void initialize_algo_management(TraderContainer& trader_container);

    static std::unique_ptr<AlgoInitializer> get_algo_initializer(shared::Mode mode);

    virtual ~AlgoInitializer() = default;
    AlgoInitializer() = default;
    AlgoInitializer(const AlgoInitializer&) = delete;
    AlgoInitializer(AlgoInitializer&&) = delete;
    AlgoInitializer& operator=(const AlgoInitializer&) = delete;
    AlgoInitializer& operator=(AlgoInitializer&&) = delete;
};

} // namespace nutc::exchange
