def place_market_order(side: str, ticker: str, quantity: float, price: float) -> None:
    """Place a market order - DO NOT MODIFY"""


class Strategy:
    """Template for a strategy."""

    def __init__(self) -> None:
        """Your initialization code goes here."""
        place_market_order("BUY", "TSLA", 10, 102)

    
    def on_trade_and_account_update(
        self, ticker: str, side: str, price: float, quantity: float, capital_remaining: float=None,
    ) -> None:
        """Called whenever two orders match or one of your orders is filled.
        Could be one of your orders, or two other people's orders.

        Parameters
        ----------
        ticker
            Ticker of orders that were matched
        side
            Side (buy/sell)
        price
            Price order executed at
        quantity
            Volume traded or of order fulfilled
        capital_remaining
            Amount of capital after fulfilling order
        """

        if isinstance(capital_remaining, type(None)):
            print(
                f"Python Account update: {ticker} {side} {price} {quantity} {capital_remaining}"
            )
        else:
            print(f"Python Trade update: {ticker} {side} {price} {quantity}")
            if ticker == "TSLA" and quantity >= 10:
                place_market_order("BUY", "APPL", 1, 100)

    def on_orderbook_update(
        self, ticker: str, side: str, price: float, quantity: float, client_id: str=None
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
