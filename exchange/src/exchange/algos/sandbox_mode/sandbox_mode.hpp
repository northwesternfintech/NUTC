#pragma once

#include "exchange/algos/algo_manager.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace algos {

class SandboxAlgoInitializer : public AlgoInitializer {
    const std::string USER_ID;
    const std::string ALGO_ID;

public:
    SandboxAlgoInitializer(std::string user_id, std::string algo_id) :
        USER_ID(std::move(user_id)), ALGO_ID(std::move(algo_id))
    {}

    void initialize_trader_container(traders::TraderContainer& traders) const override;

    void
    initialize_files() final
    {}
};

} // namespace algos
} // namespace nutc
