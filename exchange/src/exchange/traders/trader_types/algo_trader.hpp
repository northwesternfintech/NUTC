#pragma once

#include "common/types/algorithm.hpp"
#include "common/types/decimal.hpp"
#include "exchange/wrappers/handle/wrapper_handle.hpp"
#include "generic_trader.hpp"

#include <fmt/format.h>

namespace nutc::exchange {

class AlgoTrader : public GenericTrader {
    const std::string DISPLAY_NAME;
    std::optional<WrapperHandle> wrapper_handle_;

public:
    explicit AlgoTrader(
        const common::algorithm_variant& algo_variant, common::decimal_price capital
    ) :
        GenericTrader(common::get_id(algo_variant), capital),
        DISPLAY_NAME(common::get_id(algo_variant)),
        wrapper_handle_(std::make_optional<WrapperHandle>(algo_variant))
    {}

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
