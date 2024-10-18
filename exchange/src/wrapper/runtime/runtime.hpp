#pragma once

#include "common/logging/logging.hpp"
#include "common/types/decimal.hpp"
#include "common/types/ticker.hpp"
#include "common/util.hpp"
#include "wrapper/messaging/exchange_communicator.hpp"

#include <string>

namespace nutc::wrapper {
// TODO: PrintLn function should use dependency injection
// Then we can test printing functionality as well
class Runtime {
public:
    virtual ~Runtime() = default;

    Runtime(const Runtime&) = default;
    Runtime(Runtime&&) noexcept = default;
    Runtime& operator=(const Runtime&) = default;
    Runtime& operator=(Runtime&&) noexcept = default;

    Runtime(
        std::string algo, std::string trader_id, ExchangeCommunicator communicator
    ) :
        algo_(std::move(algo)), trader_id_(std::move(trader_id)),
        communicator_(std::move(communicator))
    {}

    virtual void fire_on_trade_update(
        Ticker ticker, Side side, decimal_quantity quantity, decimal_price price
    ) const = 0;
    virtual void fire_on_orderbook_update(
        Ticker ticker, Side side, decimal_quantity quantity, decimal_price price
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

    static void
    log_text(const std::string& text)
    {
        // log_i(algo_print, "{}", text);
    }

    static void
    log_error(const std::string& text)
    {
        // log_e(ALGO_ERROR, "{}", text);
    }
};

} // namespace nutc::wrapper
