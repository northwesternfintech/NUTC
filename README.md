
# Northwestern Trading Competition Monorepo

## Introduction

The Northwestern Trading Competition (NUTC) is a contest where participants submit trading algorithms to compete in a simulated market environment. These algorithms are built from a provided template and interact with real-time simulated market data, executing arbitrary stock transactions. The objective is to maximize profit through strategic buying and selling of stocks.

This document outlines the structure and key functionalities of the five fundamental components that make up the NUTC.

## Development Documentation

For detailed development documentation for NU Fintech, please refer to this [Google Docs link](https://docs.google.com/document/d/1yd7IGoILPUgHMdyfE8xAfjnyZ0ktbUPt0k9BOclE-oA/edit?usp=sharing).

## Components

### Algo Wrapper

The wrapper manages a single user-submitted algorithm within a Python interpreter (pybind). It acts as an intermediary between the algorithm and the exchange, receiving market information from the exchange and forwarding it to the algorithm via callbacks. The wrapper also binds a Python function (`place_market_order`) to a C++ function that sends market order messages to the exchange.

Each algorithm operates within its own process, ensuring isolation from the exchange and other algorithms. This setup enhances control over the algorithm, allowing limitations on certain functions and rate limiting order calls.

Location: `NUTC/exchange/src/wrapper`

Note: The wrapper code is some of the oldest in the codebase and is of low quality. Contributions to its improvement are welcome.

### Exchange

The exchange serves as the core of the NUTC and is the largest component. It performs the following key tasks:

- Spawns a wrapper process for each participant's algorithm.
- Handles the processing of market orders on each tick of the simulated exchange.
- Matches all existing market orders.
- Sends updates for matched orders.
- Displays a dashboard with metrics for performance, bots, traders, etc.

Communication between the exchange and the algorithm wrappers is facilitated through RabbitMQ, with frequent messages including market orders, order book updates, and matches.

Bots (market makers, retail traders, etc.) run within the exchange itself and interact through function calls rather than messages.

### Web Portal

The NUTC web portal allows participants to view competition information, submit algorithms, and access extensive results from both testing and live competitions.
