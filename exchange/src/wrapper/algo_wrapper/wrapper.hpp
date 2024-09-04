#pragma once

#include "common/types/decimal.hpp"
#include "common/types/ticker.hpp"
#include "common/util.hpp"
#include "wrapper/messaging/exchange_communicator.hpp"

#include <string>

namespace nutc::wrapper {

class Wrapper {
public:
    virtual ~Wrapper() = default;

    Wrapper(const Wrapper&) = default;
    Wrapper(Wrapper&&) noexcept = default;
    Wrapper& operator=(const Wrapper&) = default;
    Wrapper& operator=(Wrapper&&) noexcept = default;

    Wrapper(
        std::string algo, std::string trader_id, ExchangeCommunicator communicator
    ) :
        algo_(std::move(algo)), trader_id_(std::move(trader_id)),
        communicator_(std::move(communicator))
    {}

    virtual void fire_on_trade_update(
        Ticker ticker, Side side, decimal_price price, decimal_quantity quantity
    ) const = 0;
    virtual void fire_on_orderbook_update(
        Ticker ticker, Side side, decimal_price price, decimal_quantity quantity
    ) const = 0;
    virtual void fire_on_account_update(
        Ticker ticker, Side side, decimal_price price, decimal_quantity quantity,
        decimal_price buyer_capital
    ) const = 0;

    void main_event_loop();

    template <typename T>
    void process_message(T&& message);

    const std::string&
    get_trader_id() const
    {
        return trader_id_;
    }

protected:
    std::string algo_;
    std::string trader_id_;
    ExchangeCommunicator communicator_;
};

} // namespace nutc::wrapper
