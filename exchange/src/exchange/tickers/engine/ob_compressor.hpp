#pragma once

#include "shared/messages_exchange_to_wrapper.hpp"

#include <vector>

namespace nutc {
namespace matching {
using messages::ObUpdate;

// We have many order book updates before we actually send them out
// For example, BUY P=3 Q=5
// Then BUY P=3 Q=1
// This can be compressed into BUY P=3 Q=6
// Because the market is very centered around the midprice, this compression greatly
// reduces the number of updates THIS ASSUMES ALL OBUPDATES ARE THE SAME TICKER
class OrderbookCompressor {
    const std::string TICKER;
    std::vector<ObUpdate> updates_;

public:
    explicit OrderbookCompressor(std::string ticker) : TICKER(std::move(ticker)) {}

    void
    add_update(const ObUpdate& update)
    {
        updates_.push_back(update);
    }

    std::vector<ObUpdate>
    compress_and_get_updates()
    {
        std::unordered_map<double, double> ask_price_to_quantity;
        std::unordered_map<double, double> bid_price_to_quantity;
        for (const auto& update : updates_) {
            if (update.side == messages::SIDE::BUY) {
                bid_price_to_quantity[update.price] += update.quantity;
            }
            else {
                ask_price_to_quantity[update.price] += update.quantity;
            }
        }
        std::vector<ObUpdate> compressed_updates;
        compressed_updates.reserve(
            ask_price_to_quantity.size() + bid_price_to_quantity.size()
        );
        for (const auto& [price, quantity] : bid_price_to_quantity) {
            compressed_updates.push_back(
                ObUpdate{TICKER, messages::SIDE::BUY, price, quantity}
            );
        }
        for (const auto& [price, quantity] : ask_price_to_quantity) {
            compressed_updates.push_back(
                ObUpdate{TICKER, messages::SIDE::SELL, price, quantity}
            );
        }
        return compressed_updates;
    }

    void
    clear()
    {
        updates_.clear();
    }
};

} // namespace matching
} // namespace nutc
