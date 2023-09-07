#pragma once
//
// Created by echav on 9/4/2023.
//

#include <string>

#ifndef NUTC24_MATCH_H
#  define NUTC24_MATCH_H

namespace nuft {

class Match {
public:
    std::string ticker;
    double timestamp;
    float price;
    float quantity;
};

} // namespace nuft

#endif // NUTC24_MATCH_H
