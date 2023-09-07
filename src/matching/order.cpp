//
// Created by echav on 9/3/2023.
//
#include "order.h"

#include <string>

namespace nuft {
Order::Order(std::string tic, std::string typ, bool b, float pri) :
    ticker(tic), type(typ), buy(b), price(pri){}
}
