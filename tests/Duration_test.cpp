//
// Created by Dániel Molnár on 2019-05-01.
//

#include <Utils/TestUtil.hpp>
#include <DateTime/Time.hpp>

#include <gtest/gtest.h>

using namespace Core;

TEST(Duration, wrapper)
{
    using namespace std::chrono_literals;
    Duration five_sec = 5000ms;
    EXPECT_EQ(5s, five_sec);
    EXPECT_EQ(10s, five_sec * 2);
}

TEST(Duration, add_duration)
{
    using namespace std::chrono_literals;
    
    Duration five_sec = 5s;

    Time time;
    time += five_sec;
    
    EXPECT_EQ(time, Time(0, 0, 5, 0));
    
    time += 10.6s;
    EXPECT_EQ(time, Time(0, 0, 15, 600));
    
    time += 1428.95885min;
    EXPECT_EQ(time, Time(23, 49, 13, 132));
    EXPECT_EQ(time + 5min, Time(23, 54, 13, 132));
}

TEST(Duration, subtract_duration)
{
    using namespace std::chrono_literals;
    
    Time time(12, 45);
    
    EXPECT_EQ(time - 45min, Time(12, 0));
    EXPECT_EQ(time - 55min - 12s, Time(11, 49, 48));
}

TEST(Duration, duration_between_times)
{
    using namespace std::chrono_literals;
    
    Time quarter_to_one(12, 45);
    
    Time quarter_past_one(13, 15, 30);
    
    auto difference = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<60>>>(quarter_past_one - quarter_to_one);
    EXPECT_EQ(difference, 30.5min);
    EXPECT_EQ(quarter_past_one - quarter_to_one, 1830s);
    
    auto difference_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(quarter_past_one - quarter_to_one);
    EXPECT_EQ(difference_in_ms, 1830000ms);
    
    EXPECT_EQ(quarter_to_one - quarter_past_one, -1830s);
}
