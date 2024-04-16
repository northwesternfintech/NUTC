#pragma once

#include "exchange/algos/algo_manager.hpp"
#include "exchange/traders/trader_manager.hpp"

namespace nutc {
namespace algos {

class SandboxAlgoInitializer : public AlgoInitializer {
    const std::string user_id_;
    const std::string algo_id_;

public:
    SandboxAlgoInitializer(std::string& user_id, std::string& algo_id) :
        user_id_(user_id), algo_id_(algo_id)
    {}

    void initialize_trader_container(manager::TraderManager& traders) const final;

    void
    initialize_files() final
    {}
};

} // namespace algos
} // namespace nutc
