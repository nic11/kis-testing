#include "LeapTestCase.h"

#include <Functions.h>

TEST_F(LeapTestCase, InvalidYear) {
    EXPECT_THROW({
        IsLeap(0);
    }, std::invalid_argument);
}

TEST_F(LeapTestCase, NoDiv4) {
    EXPECT_FALSE(IsLeap(1337));
}

TEST_F(LeapTestCase, Div4) {
    EXPECT_TRUE(IsLeap(2020));
}

TEST_F(LeapTestCase, Div100) {
    EXPECT_FALSE(IsLeap(1900));
}

TEST_F(LeapTestCase, Div400) {
    EXPECT_TRUE(IsLeap(2000));
}
