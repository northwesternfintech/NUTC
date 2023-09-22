def initialize() -> None:
    print("Initializing...")
    # initialization logic goes here
    # you can call constructors, etc.

# Called whenever two orders match. Could be one of your orders, or two other people's orders.
def on_trade_update(ticker: str, side: str, price: float, quantity: float) -> None:
    print(f"Python Trade update: {ticker} {side} {price} {quantity}")

# Called whenever the orderbook changes. This could be because of a trade, or because of a new order, or both.
def on_orderbook_update(ticker: str, side: str, price: float, quantity: bool) -> None:
    print(f"Python Orderbook update: {ticker} {side} {price} {quantity}")

# Called whenever one of your orders is filled.
def on_account_update(ticker: str, side: str, price: float, quantity: float, capital_remaining: float) -> None:
    print(f"Python Account update: {ticker} {side} {price} {quantity} {capital_remaining}")
