from enum import Enum

class Side(Enum):
    BUY = 0
    SELL = 1

class Ticker(Enum):
    ETH = 0
    BTC = 1
    LTC = 2

def place_market_order(side: Side, ticker: Ticker, quantity: float) -> None:
    return

def place_limit_order(side: Side, ticker: Ticker, quantity: float, price: float, ioc: bool = False) -> int:
    return 0

def cancel_order(ticker: Ticker, order_id: int) -> int:
    return 0

class Strategy:
    """Template for a strategy."""

    def __init__(self) -> None:
        """Your initialization code goes here."""
        place_limit_order(Side.BUY, Ticker.ETH, 100.0, 10.0);

    def on_trade_update(self, ticker: Ticker, side: Side, quantity: float, price: float) -> None:
        """Called whenever two orders match. Could be one of your orders, or two other people's orders.

        Parameters
        ----------
        ticker
            Ticker of orders that were matched
        side

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
            Ticker that has an orderbook update
        side
            Which orderbook was updated
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
        quantity: float,
        price: float,
        capital_remaining: float,
    ) -> None:
        """Called whenever one of your orders is filled.

        Parameters
        ----------
        ticker
            Ticker of order that was fulfilled
        side
            Side of order that was fulfilled
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
