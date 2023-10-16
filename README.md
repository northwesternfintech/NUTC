# NUTC Monorepo

This monorepo holds all relevant repositories for the Northwestern Trading Competition project. The details of each are described below.

## Exchange
The exchange is the core of the trading competition. It handles spawning in algorithm wrappers, receiving and matching orders, and many other tasks.

## Client
The client is a wrapper over user-submitted (currently Python) algorithms. It exposes a function for algorithms to make stock purchases on the exchange, and executes callbacks when certain events occur (such as an order executing).

## Analyzer
The exchange creates a log detailing events that occurred during the competition, such as order placement, execution, etc. The analyzer consumes this log and creates metrics that are displayable for web visualizers.

## Linter
The linter is the primary way of testing user-submitted algorithms before the contest. It is (currently) designed to operate in a google cloud run environment, being triggered by user algorithm uploads.

## Web
NUTC's web interface. It includes application, authentication, algorithm submission, and more.

# Building
To initialize and build all repos, follow the steps below
1. Conan
- `pip install conan`
- `dnf install cmake`
- `dnf install go-task`
- `dnf install perl`
- `conan profile detect`
- Copy `.conan2/profiles/default` to `.conan2/profiles/cpp20` 
- Change compiler.cppstd from gnu14 to gnu20
- Change compiler.version from 8 to 13 on Linux, or to 16 on Mac
2. `task init-all`
