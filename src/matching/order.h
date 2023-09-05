//
// Created by echav on 9/3/2023.
//
#include <string>

#ifndef NUTC24_ORDER_H
#define NUTC24_ORDER_H

namespace nuft {

    class Order {
    public: //yea not sure where the defined interface is
        std::string ticker;
        std::string type;
        bool buy;
        float price;

        Order(std::string tic, std::string typ, bool b, float pri);
    };

} //namespace nuft

#endif //NUTC24_ORDER_H
