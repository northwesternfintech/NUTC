#include <gtest/gtest.h>

#include "lib.hpp"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions)
{
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");

  // Expect two strings to be equal
  auto const lib = library {};
  EXPECT_EQ(lib.name, "NUTC-client");

  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
