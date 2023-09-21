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
    std::string buyer_uid;
    std::string seller_uid;
    float price;
    float quantity;
};
} // namespace matching
} // namespace nutc
