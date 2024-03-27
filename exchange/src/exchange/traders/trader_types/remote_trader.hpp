#pragma once

#include "generic_trader.hpp"

namespace nutc {
namespace manager {
class RemoteTrader : public GenericTrader {
    const std::string FULL_NAME;
    const std::string ALGO_ID;
    pid_t pid_{};

public:
    RemoteTrader(
        std::string user_id, std::string full_name, std::string algo_id, double capital
    ) :
        GenericTrader(std::move(user_id), capital),
        FULL_NAME(std::move(full_name)), ALGO_ID(std::move(algo_id))
    {}

    constexpr TraderType
    get_type() const override
    {
        return TraderType::REMOTE;
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

    const std::string&
    get_name() const
    {
        return FULL_NAME;
    }

    const std::string&
    get_algo_id() const override
    {
        return ALGO_ID;
    }
};
} // namespace manager
} // namespace nutc
