#pragma once

#include "exchange/process_spawning/wrapper_handle.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "signal.h"
#include "trader_interface.hpp"

#include <fmt/format.h>

#include <filesystem>

namespace nutc {
namespace traders {

class LocalTrader : public GenericTrader {
    const TraderType TRADER_TYPE;
    const std::string NAME;
    const std::string ALGO_ID;
    spawning::WrapperHandle wrapper_handle_{};

public:
    // Remote (firebase)
    explicit LocalTrader(
        std::string remote_uid, std::string full_name, std::string algo_id,
        double capital
    ) :
        GenericTrader(std::move(remote_uid), capital),
        TRADER_TYPE(TraderType::remote), NAME(std::move(full_name)),
        ALGO_ID(std::move(algo_id))
    {}

    // Local (algo .py on disk)
    explicit LocalTrader(std::string algo_path, double capital) :
        GenericTrader(algo_path, capital), TRADER_TYPE(TraderType::local),
        ALGO_ID(std::move(algo_path))
    {
        if (!file_ops::file_exists(get_algo_id())) [[unlikely]] {
            std::string err_str =
                fmt::format("Unable to find local algorithm file: {}", algo_path);
            throw std::runtime_error(err_str);
        }
    }

    TraderType
    get_type() const override
    {
        return TRADER_TYPE;
    }

    bool
    can_leverage() const override
    {
        return false;
    }

    const std::string&
    get_algo_id() const override
    {
        return ALGO_ID;
    }

    const std::string&
    get_name() const
    {
        assert(TRADER_TYPE == TraderType::remote);
        return NAME;
    }

    void
    send_message(const std::string& message) final
    {
        wrapper_handle_.send_message(message);
    }

    void
    set_wrapper_handle(spawning::WrapperHandle&& handle) final
    {
        wrapper_handle_ = std::move(handle);
    }
};

} // namespace traders
} // namespace nutc
