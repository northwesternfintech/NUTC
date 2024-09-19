#include <cstdint>

#include <string>

/**
 * Place a market order
 *
 * IMPORTANT:
 * You should handle the case where the order fails due to rate limiting
 * (maybe wait and try again?)
 *
 * @param side Side of the order to place ("BUY" or "SELL")
 * @param ticker Ticker of the order to place ("ETH", "BTC", or "LTC")
 * @param quantity Volume of the order to place
 *
 * @return true if order succeeded, false if order failed due to rate limiting
 */
bool
place_market_order(std::string const& side, std::string const& ticker, double quantity);

/**
 * Place a limit order
 *
 * IMPORTANT:
 * You should handle the case where the order fails due to rate limiting
 * (maybe wait and try again?)
 *
 * @param side Side of the order to place ("BUY" or "SELL")
 * @param ticker Ticker of the order to place ("ETH", "BTC", or "LTC")
 * @param quantity Volume of the order to place
 * @param price Price of the order to place
 * @param ioc Immediate or cancel
 *
 * @return true if order succeeded, false if order failed due to rate limiting
 */
std::int64_t place_limit_order(
    std::string const& side, std::string const& ticker, double quantity, double price,
    bool ioc = false
);

bool cancel_order(std::string const& ticker, std::int64_t order_id);

class Strategy {
public:
    Strategy() = default;

    /**
     * Called whenever two orders match. Could be one of your orders, or two other
     * people's orders.
     *
     * @param ticker Ticker of the orders that were matched ("ETH", "BTC", or
     * "LTC)
     * @param side Side of the orders that were matched ("BUY" or "SELL")
     * @param price Price that trade was executed at
     * @quantity quantity Volume traded
     */
    void
    on_trade_update(std::string ticker, std::string side, double quantity, double price)
    {}

    /**
     * Called whenever the orderbook changes. This could be because of a trade, or
     * because of a new order, or both.
     *
     * @param ticker Ticker that has an orderbook update ("ETH", "BTC", or "LTC")
     * @param side Which orderbook as updated ("BUY" or "SELL")
     * @param price Price of orderbook that has an update
     * @param quantity Volume placed into orderbook
     */
    void
    on_orderbook_update(
        std::string ticker, std::string side, double quantity, double price
    )
    {
        if (ticker == "ETH" && quantity < 101 && quantity > 99) {
            place_limit_order("BUY", "ETH", 100, 10);
        }
    }

    /**
     * Called whenever one of your orders is filled.
     *
     * @param ticker Ticker of order that was fulfilled ("ETH", "BTC", or "LTC")
     * @param side Side of order that was fulfilled ("BUY" or "SELL")
     * @param price Price that order was fulfilled at
     * @param quantity Amount of capital after fulfilling order
     */
    void
    on_account_update(
        std::string ticker, std::string side, double price, double quantity,
        double capital_remaining
    )
    {}
};
