#pragma once

namespace nutc::test {

struct ReturnMeanNoiseGenerator {
    static double
    generate(double mean, double)
    {
        return mean;
    }
};
} // namespace nutc::test
