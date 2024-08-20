#pragma once

#include "exchange/wrappers/handle/wrapper_handle.hpp"
#include "generic_trader.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "shared/types/decimal_price.hpp"

#include <fmt/format.h>

namespace nutc {
namespace traders {

class AlgoTrader : public GenericTrader {
    const std::string DISPLAY_NAME;
    const std::string ALGO_ID;
    std::optional<wrappers::WrapperHandle> wrapper_handle_;

public:
    // Remote (firebase)
    explicit AlgoTrader(
        std::string remote_uid, std::string algo_id, std::string full_name,
        util::decimal_price capital
    ) :
        GenericTrader(util::trader_id(remote_uid, algo_id), capital),
        DISPLAY_NAME(std::move(full_name)), ALGO_ID(algo_id),
        wrapper_handle_(std::make_optional<wrappers::WrapperHandle>(remote_uid, algo_id)
        )
    {}

    // Local (algo .py on disk)
    explicit AlgoTrader(std::string algo_path, util::decimal_price capital) :
        GenericTrader(algo_path, capital), DISPLAY_NAME(algo_path), ALGO_ID(algo_path),
        wrapper_handle_(std::make_optional<wrappers::WrapperHandle>(algo_path))
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

    std::vector<OrderVariant>
    read_orders() override
    {
        if (!wrapper_handle_) [[unlikely]]
            return {};

        std::vector<wrappers::WrapperHandle::ReadMessageVariant> incoming_messages =
            wrapper_handle_->read_messages();

        std::vector<OrderVariant> incoming_orders;
        incoming_orders.reserve(incoming_messages.size());

        std::transform(
            incoming_messages.begin(), incoming_messages.end(),
            std::back_inserter(incoming_orders),
            [](const auto& v) -> OrderVariant {
                if (std::holds_alternative<messages::market_order>(v))
                    return std::get<messages::market_order>(v);

                if (std::holds_alternative<messages::limit_order>(v))
                    return std::get<messages::limit_order>(v);

                throw std::runtime_error("Unexpected message from wrapper");
            }
        );

        return incoming_orders;
    }

    void
    process_position_change(util::position) final
    {}
};

} // namespace traders
} // namespace nutc
