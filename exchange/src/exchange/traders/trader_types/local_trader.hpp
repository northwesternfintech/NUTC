#pragma once

#include "generic_trader.hpp"

namespace nutc {
namespace traders {

class LocalTrader : public GenericTrader {
    pid_t pid_{};

public:
    explicit LocalTrader(std::string algo_path, double capital) :
        GenericTrader(std::move(algo_path), capital)
    {}

    TraderType
    get_type() const override
    {
        return TraderType::local;
    }

    bool
    can_leverage() const override
    {
        return false;
    }

    const std::string&
    get_algo_id() const override
    {
        return get_id();
    }

    void
    set_pid(const pid_t& pid) override
    {
        pid_ = pid;
    }

    pid_t
    get_pid() const override
    {
        return pid_;
    }
};

} // namespace traders
} // namespace nutc
