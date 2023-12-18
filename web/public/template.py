def place_market_order(side: str, ticker: str, quantity: float, price: float) -> bool:
    """Place a market order - DO NOT MODIFY

    Parameters
    ----------
    side
        Side of order to place ("BUY" or "SELL")
    ticker
        Ticker of order to place ("A", "B", or "C")
    quantity
        Volume of order to place
    price
        Price of order to place

    Returns
    -------
    True if order succeeded, False if order failed due to rate limiting

    ((IMPORTANT))
    You should handle the case where the order fails due to rate limiting (maybe wait and try again?)
    """

class Strategy:
    """Template for a strategy."""

    def __init__(self) -> None:
        """Your initialization code goes here."""
        pass

    def on_trade_and_account_update(self, ticker: str, side: str, price: float, quantity: float, capital_remaining: float=None) -> None:
        """Called whenever two orders match. Could be one of your orders, or two other people's orders.

        Also called when one of your orders is filled.

        Parameters
        ----------
        ticker
            Ticker of orders that were matched ("A", "B", or "C")
        side 
            Side of orders that were matched ("BUY" or "SELL")
        price
            Price that trade was executed at
        quantity
            Volume traded
        capital_remaining (default None)
            Remaining capital
        """
        print(f"Python Trade/Account update: {ticker} {side} {price} {quantity} {capital_remaining}")

    def on_orderbook_update(
        self, ticker: str, side: str, price: float, quantity: float
    ) -> None:
        """Called whenever the orderbook changes. This could be because of a trade, or because of a new order, or both.

        Parameters
        ----------
        ticker
            Ticker that has an orderbook update ("A", "B", or "C")
        side
            Which orderbook was updated ("BUY" or "SELL")
        price
            Price of orderbook that has an update
        quantity
            Volume placed into orderbook
        """
        print(f"Python Orderbook update: {ticker} {side} {price} {quantity}")

    def on_holding_change(
        self
    ):
        """Called whenever there is a change in holdings

        Parameters
        ----------
        idk fill this in later
        """
        print(f"Python Holding change")
