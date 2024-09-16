#pragma once

#include "ticker_info.hpp"

#include <vector>

namespace nutc::exchange {

class TickerMapping {
    std::vector<ticker_info> tickers;

public:
    TickerMapping(const std::vector<ticker_config>& configs, TraderContainer& traders);

    TickerMapping();

    class Iterator {
        std::size_t index_;
        typename std::vector<ticker_info>::iterator it_;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<common::Ticker, ticker_info&>;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(std::size_t index, typename std::vector<ticker_info>::iterator iter);

        value_type operator*();

        Iterator& operator++();

        bool operator!=(const Iterator& other) const;
    };

    class ConstIterator {
        std::size_t index_;
        typename std::vector<ticker_info>::const_iterator it_;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<common::Ticker, const ticker_info&>;
        using pointer = value_type*;
        using reference = value_type&;

        ConstIterator(
            std::size_t index, typename std::vector<ticker_info>::const_iterator iter
        );

        value_type operator*() const;

        ConstIterator& operator++();

        bool operator!=(const ConstIterator& other) const;
    };

    ConstIterator begin() const;
    ConstIterator end() const;
    Iterator begin();
    Iterator end();

    ticker_info& operator[](common::Ticker ticker);
    const ticker_info& operator[](common::Ticker ticker) const;

private:
    static std::vector<ticker_info> create_tickers();
    static std::vector<ticker_info>
    create_tickers(const std::vector<ticker_config>& configs, TraderContainer& traders);
};

} // namespace nutc::exchange
