#include "AddTestCase.h"

#include <limits>

#include "Functions.h"

TEST_F(AddTestCase, TestSimple) {
  EXPECT_EQ(Add(2, 3), 5);
}

TEST_F(AddTestCase, TestIntMin) {
  constexpr static auto kIntMin = std::numeric_limits<int>::min();
  EXPECT_EQ(Add(0, kIntMin), kIntMin);
}
