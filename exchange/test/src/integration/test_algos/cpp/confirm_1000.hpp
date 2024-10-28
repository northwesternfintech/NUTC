#include <cstdint>

#include <string>

enum class Side { buy = 0, sell = 1 };
enum class Ticker : std::uint8_t { ETH = 0, BTC = 1, LTC = 2 }; // NOLINT

/**
 * Place a market order
 *
 * IMPORTANT:
 * You should handle the case where the order fails due to rate limiting
 * (maybe wait and try again?)
 *
 * @param side Side of the order to place (Side::buy or Side::sell)
 * @param ticker Ticker of the order to place (Ticker::ETH, Ticker::BTC, or "LTC")
 * @param quantity Volume of the order to place
 *
 * @return true if order succeeded, false if order failed due to rate limiting
 */
bool place_market_order(Side side, Ticker ticker, float quantity);

/**
 * Place a limit order
 *
 * IMPORTANT:
 * You should handle the case where the order fails due to rate limiting
 * (maybe wait and try again?)
 *
 * @param side Side of the order to place (Side::buy or Side::sell)
 * @param ticker Ticker of the order to place (Ticker::ETH, Ticker::BTC, or "LTC")
 * @param quantity Volume of the order to place
 * @param price Price of the order to place
 * @param ioc Immediate or cancel
 *
 * @return true if order succeeded, false if order failed due to rate limiting
 */
std::int64_t place_limit_order(
    Side side, Ticker ticker, float quantity, float price, bool ioc = false
);

bool cancel_order(Ticker ticker, std::int64_t order_id);

class Strategy {
    std::size_t num_rx_ = 0;

public:
    Strategy() = default;

    /**
     * Called whenever two orders match. Could be one of your orders, or two other
     * people's orders.
     *
     * @param ticker Ticker of the orders that were matched (Ticker::ETH, Ticker::BTC,
     * or "LTC)
     * @param side Side of the orders that were matched (Side::buy or Side::sell)
     * @param price Price that trade was executed at
     * @quantity quantity Volume traded
     */
    void
    on_trade_update(Ticker ticker, Side side, float quantity, float price)
    {}

    /**
     * Called whenever the orderbook changes. This could be because of a trade, or
     * because of a new order, or both.
     *
     * @param ticker Ticker that has an orderbook update (Ticker::ETH, Ticker::BTC, or
     * "LTC")
     * @param side Which orderbook as updated (Side::buy or Side::sell)
     * @param price Price of orderbook that has an update
     * @param quantity Volume placed into orderbook
     */
    void
    on_orderbook_update(Ticker ticker, Side side, float quantity, float price)
    {
        if (ticker == Ticker::ETH && price >= 0.0)
            num_rx_ += 1;
        if (num_rx_ == 10000)
            place_limit_order(Side::buy, Ticker::ETH, 10, 100);
    }

    /**
     * Called whenever one of your orders is filled.
     *
     * @param ticker Ticker of order that was fulfilled (Ticker::ETH, Ticker::BTC, or
     * "LTC")
     * @param side Side of order that was fulfilled (Side::buy or Side::sell)
     * @param price Price that order was fulfilled at
     * @param quantity Amount of capital after fulfilling order
     */
    void
    on_account_update(
        Ticker ticker, Side side, float price, float quantity, float capital_remaining
    )
    {}
};
