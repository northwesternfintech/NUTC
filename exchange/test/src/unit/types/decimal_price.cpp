#include "shared/types/decimal.hpp"

#include <gtest/gtest.h>

using nutc::shared::decimal_price;

TEST(UnitDecimalPrice, CorrectlyHandleSubtraction)
{
    decimal_price first{1.0};
    decimal_price second{5.0};
    EXPECT_EQ(first - second, decimal_price{-4.0});
}

TEST(UnitDecimalPrice, OutOfBoundsPriceThrowsException)
{
    double out_of_max_bound = std::numeric_limits<double>::max();
    double out_of_min_bound = std::numeric_limits<double>::lowest();
    EXPECT_THROW(decimal_price{out_of_max_bound}, std::invalid_argument);
    EXPECT_THROW(decimal_price{out_of_min_bound}, std::invalid_argument);
}
