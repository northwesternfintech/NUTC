#pragma once

#include "ticker_data.hpp"

#include <vector>

namespace nutc::exchange {

class TickerContainer {
    std::vector<TickerData> tickers;

public:
    TickerContainer(
        const std::vector<ticker_config>& configs, TraderContainer& traders
    );

    TickerContainer();

    class Iterator {
        std::size_t index_;
        typename std::vector<TickerData>::iterator it_;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<common::Ticker, TickerData&>;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(std::size_t index, typename std::vector<TickerData>::iterator iter);

        value_type operator*();

        Iterator& operator++();

        bool operator!=(const Iterator& other) const;
    };

    class ConstIterator {
        std::size_t index_;
        typename std::vector<TickerData>::const_iterator it_;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<common::Ticker, const TickerData&>;
        using pointer = value_type*;
        using reference = value_type&;

        ConstIterator(
            std::size_t index, typename std::vector<TickerData>::const_iterator iter
        );

        value_type operator*() const;

        ConstIterator& operator++();

        bool operator!=(const ConstIterator& other) const;
    };

    ConstIterator begin() const;
    ConstIterator end() const;
    Iterator begin();
    Iterator end();

    TickerData& operator[](common::Ticker ticker);
    const TickerData& operator[](common::Ticker ticker) const;

private:
    static std::vector<TickerData> create_tickers();
    static std::vector<TickerData>
    create_tickers(const std::vector<ticker_config>& configs, TraderContainer& traders);

    static std::vector<BotContainer>
    create_bot_containers(
        TraderContainer& trader_container, common::Ticker ticker,
        const std::vector<bot_config>& configs
    )
    {
        std::vector<BotContainer> bot_containers;
        bot_containers.reserve(configs.size());
        for (const bot_config& bot_config : configs) {
            bot_containers.emplace_back(ticker, trader_container, bot_config);
        }
        return bot_containers;
    }
};

} // namespace nutc::exchange
