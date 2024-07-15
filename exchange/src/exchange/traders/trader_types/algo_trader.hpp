#pragma once

#include "exchange/wrappers/handle/wrapper_handle.hpp"
#include "generic_trader.hpp"
#include "shared/file_operations/file_operations.hpp"

#include <fmt/format.h>

namespace nutc {
namespace traders {

class LocalTrader : public GenericTrader {
    const std::string DISPLAY_NAME;
    const std::string ALGO_ID;
    std::unique_ptr<wrappers::WrapperHandle> wrapper_handle_;

public:
    // Remote (firebase)
    explicit LocalTrader(
        std::string remote_uid, std::string algo_id, std::string full_name,
        double capital
    ) :
        GenericTrader(util::trader_id(remote_uid, algo_id), capital),
        DISPLAY_NAME(std::move(full_name)), ALGO_ID(algo_id),
        wrapper_handle_(std::make_unique<wrappers::WrapperHandle>(remote_uid, algo_id))
    {}

    // Local (algo .py on disk)
    explicit LocalTrader(std::string algo_path, double capital) :
        GenericTrader(algo_path, capital), DISPLAY_NAME(algo_path), ALGO_ID(algo_path),
        wrapper_handle_(std::make_unique<wrappers::WrapperHandle>(algo_path))
    {
        if (!file_ops::file_exists(ALGO_ID)) [[unlikely]] {
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

    std::vector<messages::limit_order>
    read_orders() override
    {
        if (wrapper_handle_) [[likely]]
            return wrapper_handle_->read_messages();
        return {};
    }

    void
    process_position_change(util::position) final
    {}
};

} // namespace traders
} // namespace nutc
