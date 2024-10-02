#pragma once

#include "base_algorithm.hpp"

namespace nutc::common {
// TODO
class RemoteAlgorithm : public BaseAlgorithm {
    std::string id_;
    std::string algo_data_;

public:
    RemoteAlgorithm(AlgoLanguage language, std::string algo_id, std::string algo_data) :
        BaseAlgorithm{language}, id_{std::move(algo_id)},
        algo_data_{std::move(algo_data)}
    {}

    std::string
    get_algo_string() const
    {
        return algo_data_;
    }

    std::string
    get_id() const
    {
        return id_;
    }
};
} // namespace nutc::common
