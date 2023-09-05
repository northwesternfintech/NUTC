//
// Created by echav on 9/4/2023.
//

#include <vector>
#include <algorithm>
#include <iostream>

#include "engine.h"
#include "order.h"

namespace nuft {

    Engine::Engine() {
        this->bids = std::vector < Order > {};
        this->asks = std::vector < Order > {};
    }

    void Engine::add_order(Order order) {
        if (order.buy) {
            this->bids.push_back(order);
        } else {
            this->asks.push_back(order);
        }
        return;
    }

    void Engine::match() {

        std::sort(this->bids.begin(), this->bids.end(), [](const Order& a, const Order& b) {
            return a.price > b.price;
        });

        std::sort(this->asks.begin(), this->asks.end(), [](const Order& a, const Order& b) {
            return a.price < b.price;
        });

        for (auto it = this->bids.begin(); it != this->bids.end(); ) {
            for (auto jt = this->asks.begin(); jt != this->asks.end(); ) {
                if (it->price >= jt->price) {
                    std::cout << "Matched: " << it->ticker << " " << it->price << " - " << jt->ticker << " " << jt->price << "\n";
                    it = this->bids.erase(it);
                    jt = this->asks.erase(jt);
                } else {
                    ++jt;
                }
            }
            ++it;
        }
    }
}