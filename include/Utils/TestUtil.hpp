//
// Created by Dániel Molnár on 2019-03-03.
//

#pragma once
#ifndef CORE_TESTUTIL_HPP
#define CORE_TESTUTIL_HPP

#include <gtest/gtest.h>

namespace {
#define PRINTF(COLOR, ...) {\
    switch(COLOR) {\
        case testing::internal::GTestColor::COLOR_DEFAULT:\
            testing::internal::ColoredPrintf(COLOR, "[   INFO   ] ");\
            break;\
        case testing::internal::GTestColor::COLOR_YELLOW:\
            testing::internal::ColoredPrintf(COLOR, "[  WARNING ] ");\
            break;\
        case testing::internal::GTestColor::COLOR_RED:\
            testing::internal::ColoredPrintf(COLOR, "[  ERROR   ] ");\
            break;\
        case testing::internal::GTestColor::COLOR_GREEN:\
            testing::internal::ColoredPrintf(COLOR, "[  SUCCESS ] ");\
            break;\
    }\
    testing::internal::ColoredPrintf(testing::internal::GTestColor::COLOR_DEFAULT, __VA_ARGS__);\
}
}

class TestCout : public std::stringstream {
private:
    testing::internal::GTestColor _color;
public:
    explicit TestCout(testing::internal::GTestColor color) : _color(color) {}
    ~TestCout() override {
        PRINTF(_color, "%s", str().c_str());
    }
};

#define TEST_WARNING TestCout(testing::internal::GTestColor::COLOR_YELLOW)
#define TEST_INFO TestCout(testing::internal::GTestColor::COLOR_DEFAULT)
#define TEST_ERROR TestCout(testing::internal::GTestColor::COLOR_RED)
#define TEST_SUCCESS TestCout(testing::internal::GTestColor::COLOR_GREEN)

#endif //CORE_TESTUTIL_HPP
