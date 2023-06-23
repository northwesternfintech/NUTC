## api reference heh


### String orderID place_order(String symbol, String side, Float quantity, Float price, String orderType)
Params:
- symbol: the symbol of the asset you want to trade
- side: "BUY" or "SELL"
- quantity: the amount of the asset you want to trade (in units of the asset)
- price: the price you want to trade at if orderType != Market
- orderType: "Market" or "Limit"

Returns:
- orderID: the ID of the order you just placed. keep track of this to maintain it's status or cancel it.

### String status get_order_status(String orderId)
Params:
- orderId: the ID of the order you want to check the status of

Returns:
- status: the status of the order. "NEW", "PARTIALLY_FILLED", "FILLED", "CANCELED", "PENDING_CANCEL", "REJECTED", or "EXPIRED"

### Order cancel_order(String orderId)
Params:
- orderId: the ID of the order you want to cancel

Returns
- status: the status of the cancellation, or null. null means no order to cancel/order was filled. a retunred order is just the order that was cancelled - note that filled trades will be removed.  

### get_positions()
Params: none

Returns: umap of positions : quantity IN BASE UNIT - USD

### get_balance()
Params: none
Retunrs: float of base unit balance -usd

### List<Order> get_open_orders()
Params: none
Returns: list of open orders
