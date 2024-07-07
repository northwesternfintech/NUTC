def place_market_order(side: str, ticker: str, quantity: float) -> None:
    """Place a market order - DO NOT MODIFY""" 

def place_limit_order(side: str, ticker: str, price: float, quantity: float, ioc: bool = False) -> None:
    """Place a limit order - DO NOT MODIFY""" 

class Strategy:
    """Template for a strategy."""

    def __init__(self) -> None:
        """Your initialization code goes here."""
        self.num = 0

    def on_trade_update(self, ticker: str, side: str, price: float, quantity: float) -> None:
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
        self, ticker: str, side: str, price: float, quantity: float
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
        if(ticker=="ABC" and price>=0.0):
            self.num+=1
        if(self.num==100000):
            place_limit_order("BUY", "ABC", 100, 10)

    def on_account_update(
        self,
        ticker: str,
        side: str,
        price: float,
        quantity: float,
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
