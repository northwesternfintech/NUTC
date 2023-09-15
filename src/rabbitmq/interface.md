# Plumbing/Client-Related Messages

Used for coordination between the exchange and clients for initialization,
shutdown, and error handling.

- **ShutdownMessage**

  - Purpose: Signal system/component shutdown.
    - `shutdown_reason`: Reason for the shutdown.

- **RMQError**

  - Purpose: Represent RabbitMQ or related errors.
    - `message`: Specific error details.

- **InitMessage**
  - Purpose: Convey initialization status.
    - `client_uid`: Unique client identifier.
    - `ready`: Indicates if the client is ready.

# Matching Engine Messages

For managing and processing orders within the trading system.

- **MarketOrder**

  - Purpose: Submit an order to the market.
    - `client_uid`: Unique identifier for the client placing the order.
    - `side`: Trade direction, represented by the `SIDE` enum (e.g., BUY, SELL).
    - `type`: Type of the order (e.g., limit, market).
    - `ticker`: Identifier for the security being traded (e.g., stock ticker).
    - `quantity`: Amount of the security to be traded.
    - `price`: Price at which the order should be executed.

- **ObUpdate**
  - Purpose: Update the order book.
    - `client_id`: Identifier for the client placing the update.
    - `security`: The security's identifier.
    - `price`: Price point for the update.
    - `quantity`: Amount of the security involved in the update.
