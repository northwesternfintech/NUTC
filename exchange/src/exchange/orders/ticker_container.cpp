#include "ticker_container.hpp"

namespace nutc::exchange {

TickerMapping::TickerMapping(
    const std::vector<ticker_config>& configs, TraderContainer& traders,
    double order_fee
) : tickers(create_tickers(configs, traders, order_fee))
{}

TickerMapping::TickerMapping(double order_fee) : tickers(create_tickers(order_fee)) {}

TickerMapping::Iterator::Iterator(
    std::size_t index, typename std::vector<ticker_info>::iterator iter
) : index_(index), it_(iter)
{}

TickerMapping::Iterator::value_type
TickerMapping::Iterator::operator*()
{
    return {static_cast<shared::Ticker>(index_), *it_};
}

TickerMapping::Iterator&
TickerMapping::Iterator::operator++()
{
    ++it_;
    ++index_;
    return *this;
}

bool
TickerMapping::Iterator::operator!=(const Iterator& other) const
{
    return it_ != other.it_;
}

TickerMapping::ConstIterator::ConstIterator(
    std::size_t index, typename std::vector<ticker_info>::const_iterator iter
) : index_(index), it_(iter)
{}

TickerMapping::ConstIterator::value_type
TickerMapping::ConstIterator::operator*() const
{
    return {static_cast<shared::Ticker>(index_), *it_};
}

TickerMapping::ConstIterator&
TickerMapping::ConstIterator::operator++()
{
    ++it_;
    ++index_;
    return *this;
}

bool
TickerMapping::ConstIterator::operator!=(const ConstIterator& other) const
{
    return it_ != other.it_;
}

TickerMapping::ConstIterator
TickerMapping::begin() const
{
    return {0, tickers.cbegin()};
}

TickerMapping::ConstIterator
TickerMapping::end() const
{
    return {tickers.size(), tickers.cend()};
}

TickerMapping::Iterator
TickerMapping::begin()
{
    return {0, tickers.begin()};
}

TickerMapping::Iterator
TickerMapping::end()
{
    return {tickers.size(), tickers.end()};
}

ticker_info&
TickerMapping::operator[](shared::Ticker ticker)
{
    return tickers[std::to_underlying(ticker)];
}

const ticker_info&
TickerMapping::operator[](shared::Ticker ticker) const
{
    return tickers[std::to_underlying(ticker)];
}

std::vector<ticker_info>
TickerMapping::create_tickers(double order_fee)
{
    std::vector<ticker_info> result;
    for (std::size_t ticker = 0; ticker < shared::TICKERS.size(); ticker++) {
        result.emplace_back(static_cast<shared::Ticker>(ticker), order_fee);
    }
    return result;
}

std::vector<ticker_info>
TickerMapping::create_tickers(
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

} // namespace nutc::exchange
