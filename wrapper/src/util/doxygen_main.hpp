#pragma once

/**
@mainpage NUTC Client Documentation

@section intro_sec Introduction

NUTC Client serves as a Python/C++ wrapper designed to facilitate the operation of user-created trading algorithms. It is intended to be used in conjunction with the NUTC24 exchange system, providing an interface for algorithmic trading clients to interact with the exchange. Each instance of the NUTC Client can run a singular user-defined trading algorithm and exposes an API for seamless communication with the exchange that spawned it.

@section architecture Architecture

The NUTC Client comprises several core components:

- rabbitmq
- py_cpp_wrapper
- firebase

@section flow_sec Communication Flow

1. The NUTC24 exchange spawns instances of the NUTC Client, one for each trading algorithm.
2. Each instance receives order book updates, match updates, and account updates from the NUTC24 exchange.
3. NUTC Client instances can place orders back on the exchange via the API.

@subsection rabbitmq RabbitMQ Communication

RabbitMQ is used to handle real-time communication between the NUTC24 exchange and each NUTC Client instance. It ensures that order book updates, match updates, and account updates are delivered reliably.

@subsection py_cpp_wrapper Python/C++ Wrapper

The Python/C++ wrapper serves as the runtime environment where user-defined algorithms are executed. The current implementation supports Python-based algorithms with plans to add C++ support.

  - @note C++ support is marked as TODO.

@subsection firebase Firebase Fetching

Firebase is used for fetching the actual client algorithms that are run within the Python/C++ wrapper. The algorithms are stored in a Firebase database and fetched at runtime.

*/

