#pragma once

#include "base_algorithm.hpp"

namespace nutc::common {
// TODO
class RemoteAlgorithm : public BaseAlgorithm {
public:
    RemoteAlgorithm() : BaseAlgorithm(AlgoLanguage::cpp)
    {
        throw std::runtime_error("Not implemented");
    }

    static std::string
    get_algo_string()
    {
        throw std::runtime_error("Not implemented");
    }

    // todo: deprecate
    static std::string
    get_id()
    {
        throw std::runtime_error("Not implemented");
    }
};
} // namespace nutc::common
