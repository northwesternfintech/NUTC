#include "ticker_container.hpp"

#include "common/types/ticker.hpp"

namespace nutc::exchange {

TickerContainer::TickerContainer(
    const std::vector<ticker_config>& configs, TraderContainer& traders
) : tickers(create_tickers(configs, traders))
{}

TickerContainer::TickerContainer() : tickers(create_tickers()) {}

TickerContainer::Iterator::Iterator(
    std::size_t index, typename std::vector<TickerData>::iterator iter
) : index_(index), it_(iter)
{}

TickerContainer::Iterator::value_type
TickerContainer::Iterator::operator*()
{
    return {static_cast<common::Ticker>(index_), *it_};
}

TickerContainer::Iterator&
TickerContainer::Iterator::operator++()
{
    ++it_;
    ++index_;
    return *this;
}

bool
TickerContainer::Iterator::operator!=(const Iterator& other) const
{
    return it_ != other.it_;
}

TickerContainer::ConstIterator::ConstIterator(
    std::size_t index, typename std::vector<TickerData>::const_iterator iter
) : index_(index), it_(iter)
{}

TickerContainer::ConstIterator::value_type
TickerContainer::ConstIterator::operator*() const
{
    return {static_cast<common::Ticker>(index_), *it_};
}

TickerContainer::ConstIterator&
TickerContainer::ConstIterator::operator++()
{
    ++it_;
    ++index_;
    return *this;
}

bool
TickerContainer::ConstIterator::operator!=(const ConstIterator& other) const
{
    return it_ != other.it_;
}

TickerContainer::ConstIterator
TickerContainer::begin() const
{
    return {0, tickers.cbegin()};
}

TickerContainer::ConstIterator
TickerContainer::end() const
{
    return {tickers.size(), tickers.cend()};
}

TickerContainer::Iterator
TickerContainer::begin()
{
    return {0, tickers.begin()};
}

TickerContainer::Iterator
TickerContainer::end()
{
    return {tickers.size(), tickers.end()};
}

TickerData&
TickerContainer::operator[](common::Ticker ticker)
{
    return tickers[std::to_underlying(ticker)];
}

const TickerData&
TickerContainer::operator[](common::Ticker ticker) const
{
    return tickers[std::to_underlying(ticker)];
}

std::vector<TickerData>
TickerContainer::create_tickers()
{
    std::vector<TickerData> result;
    result.reserve(common::TICKERS.size());
    for (std::size_t ticker = 0; ticker < common::TICKERS.size(); ticker++) {
        result.emplace_back(static_cast<common::Ticker>(ticker));
    }
    return result;
}

std::vector<TickerData>
TickerContainer::create_tickers(
    const std::vector<ticker_config>& configs, TraderContainer& traders
)
{
    std::vector<TickerData> result;
    // this is really bad. fix soon
    std::unordered_map<std::size_t, TickerData> ticker_map;
    for (const auto& config : configs) {
        auto bots = create_bot_containers(traders, config.TICKER, config.BOTS);
        ticker_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(std::to_underlying(config.TICKER)),
            std::forward_as_tuple(config.TICKER, config.STARTING_PRICE, std::move(bots))
        );
    }
    for (std::size_t ticker = 0; ticker < common::TICKERS.size(); ticker++) {
        if (ticker_map.contains(ticker))
            result.emplace_back(ticker_map.at(ticker));
        else
            result.emplace_back(static_cast<common::Ticker>(ticker));
    }
    return result;
}

} // namespace nutc::exchange
