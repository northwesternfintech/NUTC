#pragma once
#include "exchange/bots/bot_container.hpp"
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/matching/engine.hpp"
#include "exchange/orders/orderbook/cancellable_orderbook.hpp"
#include "exchange/orders/orderbook/level_tracked_orderbook.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/types/decimal_price.hpp"

#include <absl/hash/hash.h>

namespace nutc::exchange {

using DecoratedLimitOrderBook =
    LevelTrackedOrderbook<CancellableOrderBook<LimitOrderBook>>;

/**
 * @brief Contains the canonical reference to all data coupled to a ticker. Very useful
 * because we typically have to access all at once
 */
// TODO: rename
struct ticker_info {
    DecoratedLimitOrderBook limit_orderbook;
    Engine engine;
    std::vector<BotContainer> bot_containers;

    ticker_info(shared::Ticker ticker, double order_fee) :
        limit_orderbook(ticker), engine(order_fee)
    {}

    void
    set_bot_config(TraderContainer& traders, const ticker_config& config)
    {
        bot_containers = create_bot_containers(
            traders, config.TICKER, config.STARTING_PRICE, config.BOTS
        );
    }

private:
    std::vector<BotContainer>
    create_bot_containers(
        TraderContainer& trader_container, shared::Ticker ticker,
        shared::decimal_price starting_price, const std::vector<bot_config>& configs
    )
    {
        std::vector<BotContainer> containers;
        containers.reserve(configs.size());
        for (const bot_config& bot_config : configs) {
            containers.emplace_back(
                ticker, starting_price, trader_container, bot_config
            );
        }
        return containers;
    }
};

class TickerMapping {
    std::vector<ticker_info> tickers;

public:
    TickerMapping(
        const std::vector<ticker_config>& configs, TraderContainer& traders,
        double order_fee
    ) :
        tickers(create_tickers(configs, traders, order_fee))
    {}

    TickerMapping(double order_fee) : tickers(create_tickers(order_fee)) {}

    class Iterator {
        std::size_t index_;
        typename std::vector<ticker_info>::iterator it_;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<shared::Ticker, ticker_info&>;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(std::size_t index, typename std::vector<ticker_info>::iterator iter) :
            index_(index), it_(iter)
        {}

        value_type
        operator*()
        {
            return {static_cast<shared::Ticker>(index_), *it_};
        }

        Iterator&
        operator++()
        {
            ++it_;
            ++index_;
            return *this;
        }

        bool
        operator!=(const Iterator& other) const
        {
            return it_ != other.it_;
        }
    };

    class ConstIterator {
        std::size_t index_;
        typename std::vector<ticker_info>::const_iterator it_;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<shared::Ticker, const ticker_info&>;
        using pointer = value_type*;
        using reference = value_type&;

        ConstIterator(
            std::size_t index, typename std::vector<ticker_info>::const_iterator iter
        ) :
            index_(index),
            it_(iter)
        {}

        value_type
        operator*() const
        {
            return {static_cast<shared::Ticker>(index_), *it_};
        }

        ConstIterator&
        operator++()
        {
            ++it_;
            ++index_;
            return *this;
        }

        bool
        operator!=(const ConstIterator& other) const
        {
            return it_ != other.it_;
        }
    };

    ConstIterator
    begin() const
    {
        return {0, tickers.cbegin()};
    }

    ConstIterator
    end() const
    {
        return {tickers.size(), tickers.cend()};
    }

    Iterator
    begin()
    {
        return {0, tickers.begin()};
    }

    Iterator
    end()
    {
        return {tickers.size(), tickers.end()};
    }

    ticker_info&
    operator[](shared::Ticker ticker)
    {
        return tickers[std::to_underlying(ticker)];
    }

private:
    static std::vector<ticker_info>
    create_tickers(double order_fee)
    {
        std::vector<ticker_info> result;
        for (std::size_t ticker = 0; ticker < shared::TICKERS.size(); ticker++) {
            result.emplace_back(static_cast<shared::Ticker>(ticker), order_fee);
        }
        return result;
    }

    static std::vector<ticker_info>
    create_tickers(
        const std::vector<ticker_config>& configs, TraderContainer& traders,
        double order_fee
    )
    {
        std::vector<ticker_info> result = create_tickers(order_fee);
        for (const auto& config : configs) {
            result[std::to_underlying(config.TICKER)].set_bot_config(traders, config);
        }
        return result;
    }
};

} // namespace nutc::exchange
