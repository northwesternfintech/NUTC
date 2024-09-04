#pragma once

#include "common/file_operations/file_operations.hpp"
#include "common/types/decimal.hpp"
#include "exchange/wrappers/handle/wrapper_handle.hpp"
#include "generic_trader.hpp"

#include <fmt/format.h>

namespace nutc::exchange {

class AlgoTrader : public GenericTrader {
    const std::string DISPLAY_NAME;
    const std::string ALGO_ID;
    std::optional<WrapperHandle> wrapper_handle_;

public:
    // Remote (firebase)
    explicit AlgoTrader(
        std::string remote_uid, std::string algo_id, std::string full_name,
        common::decimal_price capital, WrapperHandle wrapper_handle
    ) :
        GenericTrader(common::trader_id(remote_uid, algo_id), capital),
        DISPLAY_NAME(std::move(full_name)), ALGO_ID(algo_id),
        wrapper_handle_(std::move(wrapper_handle))
    {}

    // Local (algo .py on disk)
    explicit AlgoTrader(
        std::string algo_path, common::decimal_price capital,
        WrapperHandle wrapper_handle
    ) :
        GenericTrader(algo_path, capital), DISPLAY_NAME(algo_path), ALGO_ID(algo_path),
        wrapper_handle_(std::move(wrapper_handle))
    {
        if (!common::file_exists(ALGO_ID)) [[unlikely]] {
            std::string err_str =
                fmt::format("Unable to find local algorithm file: {}", algo_path);
            throw std::runtime_error(err_str);
        }
    }

    const std::string&
    get_type() const override
    {
        static const std::string TYPE = "ALGO";
        return TYPE;
    }

    const std::string&
    get_display_name() const override
    {
        return DISPLAY_NAME;
    }

    bool
    can_leverage() const override
    {
        return false;
    }

    void
    disable() override
    {
        wrapper_handle_.reset();
    }

    void
    send_message(const std::string& message) final
    {
        if (wrapper_handle_) [[likely]]
            wrapper_handle_->send_message(message);
    }

    IncomingMessageQueue
    read_orders() override
    {
        if (!wrapper_handle_.has_value()) [[unlikely]]
            return {};

        return wrapper_handle_->read_shared();
    }

    void
    notify_position_change(common::position) final
    {}
};

} // namespace nutc::exchange
