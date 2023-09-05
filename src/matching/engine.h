//
// Created by echav on 9/4/2023.
//

#ifndef NUTC24_ENGINE_H
#define NUTC24_ENGINE_H

#include <vector>
#include "order.h"

namespace nuft {

    class Engine {
    public: //we will need to add all communication machinery in, this will just expose
        std::vector <Order> bids;
        std::vector <Order> asks;

        Engine(); //con
        void add_order(Order order);
        void match();
    };

} //namespace nuft


#endif //NUTC24_ENGINE_H
