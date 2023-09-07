//
// Created by echav on 9/3/2023.
//
#include "order.hpp"

#include <string>

namespace nutc {
namespace matching {
Order::Order(std::string tic, std::string typ, bool b, float pri) :
    ticker(tic), type(typ), buy(b), price(pri)
{}
} // namespace matching
} // namespace nuft
