#pragma once

#include "exchange/wrappers/handle/wrapper_handle.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "trader_interface.hpp"

#include <fmt/format.h>

namespace nutc {
namespace traders {

class LocalTrader : public GenericTrader {
    const std::string DISPLAY_NAME;
    const std::string ALGO_ID;
    wrappers::WrapperHandle wrapper_handle_;

public:
    // Remote (firebase)
    explicit LocalTrader(
        std::string remote_uid, std::string algo_id, std::string full_name,
        double capital
    ) :
        GenericTrader(util::trader_id(remote_uid, algo_id), capital),
        DISPLAY_NAME(std::move(full_name)), ALGO_ID(algo_id),
        wrapper_handle_(remote_uid, algo_id)
    {}

    // Local (algo .py on disk)
    explicit LocalTrader(std::string algo_path, double capital) :
        GenericTrader(algo_path, capital), DISPLAY_NAME(algo_path), ALGO_ID(algo_path),
        wrapper_handle_(algo_path)
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
        static constexpr std::string type = "ALGO";
        return type;
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
    send_message(const std::string& message) final
    {
        wrapper_handle_.send_message(message);
    }

    std::vector<market_order>
    read_orders() override
    {
        return wrapper_handle_.read_messages();
    }

    void
    process_order_remove(market_order) final
    {}

    void
    process_order_add(market_order) final
    {}
};

} // namespace traders
} // namespace nutc
