#include "common/types/decimal.hpp"

#include <gtest/gtest.h>

using nutc::common::decimal_price;

TEST(UnitDecimalPrice, ImplicitConversion)
{
    decimal_price first{5.0};
    double second{first};
    EXPECT_EQ(second, 5.0);
}

TEST(UnitDecimalPrice, AssignmentOperator)
{
    decimal_price first = 5.0;
    double second{first};
    EXPECT_EQ(second, 5.0);
}

TEST(UnitDecimalPrice, TestCommutativity)
{
    decimal_price three = 3.0;
    decimal_price four = 4.0;
    decimal_price six = 6.0;

    decimal_price twelve = three * four;
    EXPECT_EQ(double{twelve / six}, 2.0);

    decimal_price point_five = three / six;
    EXPECT_EQ(double{point_five * four}, 2.0);
}

TEST(UnitDecimalPrice, UnaryNegate)
{
    decimal_price first = 5.0;
    double second{-first};
    EXPECT_EQ(second, -5.0);
}

TEST(UnitDecimalPrice, DecimalDecimalSubtraction)
{
    decimal_price first = 5.0;
    decimal_price second = 3.0;
    double third{first - second};
    EXPECT_EQ(third, 2.0);
}

TEST(UnitDecimalPrice, DecimalDecimalAddition)
{
    decimal_price first = 5.0;
    decimal_price second = 3.0;
    double third{first + second};
    EXPECT_EQ(third, 8.0);
}

TEST(UnitDecimalPrice, DecimalDecimalDivision)
{
    decimal_price first = 10.0;
    decimal_price second = 2.0;
    decimal_price third = first / second;
    EXPECT_EQ(double{third}, 5.0);
}

TEST(UnitDecimalPrice, DecimalDecimalMultiplication)
{
    decimal_price first = 10.0;
    decimal_price second = 2.0;
    double third{first * second};
    EXPECT_EQ(third, 20.0);
}

TEST(UnitDecimalPrice, DecimalDecimalMultiplicationAssignment)
{
    decimal_price first = 10.0;
    decimal_price second = 2.0;
    first *= second;
    double third{first};
    EXPECT_EQ(third, 20.0);
}

TEST(UnitDecimalPrice, DecimalDecimalAdditionAssignment)
{
    decimal_price first = 10.0;
    decimal_price second = 2.0;
    first += second;
    double third{first};
    EXPECT_EQ(third, 12.0);
}

TEST(UnitDecimalPrice, DecimalDecimalEquality)
{
    decimal_price first = 10.0;
    decimal_price second = 2.0;
    decimal_price third = 10.0;
    EXPECT_FALSE(first == second);
    EXPECT_TRUE(first == third);
}

TEST(UnitDecimalPrice, CorrectlyHandleNegativeSubtraction)
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
