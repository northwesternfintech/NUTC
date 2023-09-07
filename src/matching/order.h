#pragma once
//
// Created by echav on 9/3/2023.
//
#include <string>

namespace nuft {
namespace matching {

class Order {
public: // yea not sure where the defined interface is
    std::string ticker;
    std::string type;
    bool buy;
    float price;

    Order(std::string tic, std::string typ, bool b, float pri);
};
} // namespace matching
} // namespace nuft
