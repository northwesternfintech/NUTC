//
// Created by echav on 9/4/2023.
//

#include "engine.hpp"

#include "order.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace nutc {
namespace matching {

Engine::Engine()
{
    this->bids = std::vector<Order>{};
    this->asks = std::vector<Order>{};
}

void
Engine::add_order(Order order)
{
    if (order.buy) {
        this->bids.push_back(order);
    }
    else {
        this->asks.push_back(order);
    }
    return;
}

std::vector<Match>
Engine::match()
{
    std::vector<Match> matches;
    std::sort(this->bids.begin(), this->bids.end(), [](const Order& a, const Order& b) {
        return a.price > b.price;
    });

    std::sort(this->asks.begin(), this->asks.end(), [](const Order& a, const Order& b) {
        return a.price < b.price;
    });

    for (auto it = this->bids.begin(); it != this->bids.end();) {
        for (auto jt = this->asks.begin(); jt != this->asks.end();) {
            if (it->price >= jt->price) {
                std::cout << "Matched: " << it->ticker << " " << it->price << " - "
                          << jt->ticker << " " << jt->price << "\n";
                auto now = std::chrono::system_clock::now().time_since_epoch();
                double timestamp =
                    std::chrono::duration<double, std::milli>(now).count();
                // TODO ETHAN: make sure passive price is used, also set up for partial
                // fills
                matches.push_back(Match{it->ticker, timestamp, jt->price, jt->quantity}
                );
                it = this->bids.erase(it);
                jt = this->asks.erase(jt);
            }
            else {
                ++jt;
            }
        }
        ++it;
    }
    return matches;
}
} // namespace matching
} // namespace nutc
