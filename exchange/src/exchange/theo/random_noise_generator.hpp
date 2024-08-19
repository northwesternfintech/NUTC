#pragma once

#include <random>

namespace nutc::stochastic {

struct RandomNoiseGenerator {
    static double
    generate(double mean, double stddev)
    {
        static std::mt19937 gen{std::random_device{}()};
        return std::normal_distribution<double>{mean, stddev}(gen);
    }
};
} // namespace nutc::stochastic
