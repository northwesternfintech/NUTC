#pragma once

#include "generic_trader.hpp"

namespace nutc {
namespace manager {
class RemoteTrader : public GenericTrader {
public:
    RemoteTrader(std::string user_id, std::string algo_id, double capital) :
        GenericTrader(std::move(user_id), capital), algo_id_(std::move(algo_id))
    {}

    constexpr TraderType
    get_type() const override
    {
        return TraderType::REMOTE;
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

    std::string
    get_algo_id() const override
    {
        return algo_id_;
    }

private:
    std::string algo_id_;
    pid_t pid_{};
};
} // namespace manager
} // namespace nutc
