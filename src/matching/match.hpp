#pragma once
//
// Created by echav on 9/4/2023.
//

#include <string>

namespace nutc {
namespace matching {

class Match {
public:
    std::string ticker;
    double timestamp;
    float price;
    float quantity;
};
} // namespace matching
} // namespace nutc
