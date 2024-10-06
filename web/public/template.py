from enum import Enum

class Side(Enum):
    BUY = 0
    SELL = 1

class Ticker(Enum):
    ETH = 0
    BTC = 1
    LTC = 2

def place_market_order(side: Side, ticker: Ticker, quantity: float) -> bool:
    """Place a market order - DO NOT MODIFY

    Parameters
    ----------
    side
        Side of order to place (Side.BUY or Side.SELL)
    ticker
        Ticker of order to place (Ticker.ETH, Ticker.BTC, or "LTC")
    quantity
        Volume of order to place

    Returns
    -------
    True if order succeeded, False if order failed due to rate limiting

    ((IMPORTANT))
    You should handle the case where the order fails due to rate limiting (maybe wait and try again?)
    """
    return True

def place_limit_order(side: Side, ticker: Ticker, quantity: float, price: float, ioc: bool = False) -> int:
    """Place a limit order - DO NOT MODIFY

    Parameters
    ----------
    side
        Side of order to place (Side.BUY or Side.SELL)
    ticker
        Ticker of order to place (Ticker.ETH, Ticker.BTC, or "LTC")
    quantity
        Volume of order to place
    price
        Price of order to place

    Returns
    -------
    order_id if order succeeded, -1 if order failed due to rate limiting
    """
    return 0

def cancel_order(ticker: Ticker, order_id: int) -> bool:
    """Place a limit order - DO NOT MODIFY
    Parameters
    ----------
    ticker
        Ticker of order to place (Ticker.ETH, Ticker.BTC, or "LTC")
    order_id
        order_id returned by place_limit_order

    Returns
    -------
    True if order succeeded, False if cancellation failed due to rate limiting
    """
    return True

# You can use print() and view the logs after sandbox run has completed
# Might help for debugging
class Strategy:
    """Template for a strategy."""

    def __init__(self) -> None:
        """Your initialization code goes here."""

    def on_trade_update(self, ticker: Ticker, side: Side, quantity: float, price: float) -> None:
        """Called whenever two orders match. Could be one of your orders, or two other people's orders.

        Parameters
        ----------
        ticker
            Ticker of orders that were matched (Ticker.ETH, Ticker.BTC, or "LTC")
        side 
            Side of orders that were matched (Side.BUY or Side.SELL)
        price
            Price that trade was executed at
        quantity
            Volume traded
        """
        print(f"Python Trade update: {ticker} {side} {price} {quantity}")

    def on_orderbook_update(
        self, ticker: Ticker, side: Side, quantity: float, price: float
    ) -> None:
        """Called whenever the orderbook changes. This could be because of a trade, or because of a new order, or both.

        Parameters
        ----------
        ticker
            Ticker that has an orderbook update (Ticker.ETH, Ticker.BTC, or "LTC")
        side
            Which orderbook was updated (Side.BUY or Side.SELL)
        price
            Price of orderbook that has an update
        quantity
            Volume placed into orderbook
        """
        print(f"Python Orderbook update: {ticker} {side} {price} {quantity}")

    def on_account_update(
        self,
        ticker: Ticker,
        side: Side,
        price: float,
        quantity: float,
        capital_remaining: float,
    ) -> None:
        """Called whenever one of your orders is filled.

        Parameters
        ----------
        ticker
            Ticker of order that was fulfilled (Ticker.ETH, Ticker.BTC, or "LTC")
        side
            Side of order that was fulfilled (Side.BUY or Side.SELL)
        price
            Price that order was fulfilled at
        quantity
            Volume of order that was fulfilled
        capital_remaining
            Amount of capital after fulfilling order
        """
        print(
            f"Python Account update: {ticker} {side} {price} {quantity} {capital_remaining}"
        )
