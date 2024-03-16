#pragma once

#include "generic_trader.hpp"

namespace nutc {
namespace manager {

class LocalTrader : public GenericTrader {
public:
    explicit LocalTrader(std::string algo_path, double capital) :
        GenericTrader(std::move(algo_path), capital)
    {}

    constexpr TraderType
    get_type() const override
    {
        return TraderType::LOCAL;
    }

    std::string
    get_algo_id() const override
    {
        return get_id();
    }

    void
    set_pid(pid_t pid) override
    {
        pid_ = pid;
    }

    pid_t
    get_pid() const override
    {
        return pid_;
    }

private:
    pid_t pid_{};
};

} // namespace manager
} // namespace nutc
