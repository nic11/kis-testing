#include "MonthDaysTestCase.h"

#include <Functions.h>

TEST_F(MonthDaysTestCase, InvalidMonth) {
  EXPECT_THROW({
      GetMonthDays(2020, 0);
  }, std::invalid_argument);
  EXPECT_THROW({
      GetMonthDays(2020, 13);
  }, std::invalid_argument);
}

TEST_F(MonthDaysTestCase, FebruaryLeap) {
  EXPECT_EQ(GetMonthDays(2020, 2), 29);
}

TEST_F(MonthDaysTestCase, FebruaryNotLeap) {
  EXPECT_EQ(GetMonthDays(2021, 2), 28);
}

TEST_F(MonthDaysTestCase, Year2020) {
  const static std::vector<int> kExpectedValues{
      31, 29, 31, // Jan-Mar
      30, 31, 30, // Apr-Jun
      31, 31, 30, // Jul-Sep
      31, 30, 31, // Oct-Dec
  };
  std::vector<int> result;
  for (int month = 1; month <= 12; ++month) {
    result.emplace_back(GetMonthDays(2020, month));
  }
  EXPECT_EQ(result, kExpectedValues);
}
