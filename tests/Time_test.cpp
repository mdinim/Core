//
// Created by Dániel Molnár on 2019-05-01.
//

#include <Utils/TestUtil.hpp>
#include <DateTime/Time.hpp>

#include <gtest/gtest.h>

using namespace Core;

TEST(Time, can_be_constructed)
{
    Time time;

    ASSERT_EQ(time.hour(), 0);
    ASSERT_EQ(time.minute(), 0);
    ASSERT_EQ(time.second(), 0);
    ASSERT_EQ(time.millisecond(), 0);
}

TEST(Time, getters)
{
    Time time(12, 59, 45, 99);
    
    EXPECT_EQ(time.hour(), 12);
    EXPECT_EQ(time.minute(), 59);
    EXPECT_EQ(time.second(), 45);
    EXPECT_EQ(time.millisecond(), 99);
    
    EXPECT_NE(time, Time(20, 42, 12, 521));
    EXPECT_EQ(time, Time(12, 59, 45, 99));
}

TEST(Time, overflow)
{
    Time time(13);
    
    EXPECT_EQ(time.hour(), 13);
}

TEST(Time, copy)
{
    Time lhs(9, 5, 3, 2);
    Time rhs;
    
    ASSERT_EQ(rhs, Time(0, 0, 0, 0));
    
    rhs = lhs;
    
    ASSERT_EQ(lhs, rhs);
}

TEST(Time, equality)
{
    using namespace std::chrono_literals;
    
    Time first(12, 2, 4, 5);
    Time second = first + 24h + 15min;
    ASSERT_EQ(first + 15min, second);
}

TEST(Time, output_operator)
{
    Time time(9, 24, 5, 333);
    std::stringstream stream;
    
    stream << Time::iso;
    
    auto reset_stream = [&stream] () {
        stream.str(std::string());
    };
    stream << time;
    
    // Default layout
    EXPECT_EQ(stream.str(), "09:24:05.333");
    
    // Set hour only
    reset_stream();
    stream << Time::hour << time;
    EXPECT_EQ(stream.str(), "09");
    
    reset_stream();
    stream << Time::minute << time;
    EXPECT_EQ(stream.str(), "09:24");
    
    reset_stream();
    stream << Time::nohour << time;
    EXPECT_EQ(stream.str(), "24");
    
    reset_stream();
    stream << Time::second << time;
    EXPECT_EQ(stream.str(), "24:05");
    
    reset_stream();
    stream << Time::nominute
    << time;
    EXPECT_EQ(stream.str(), "05");
    
    reset_stream();
    stream << Time::millisecond << time;
    EXPECT_EQ(stream.str(), "05.333");
    
    reset_stream();
    stream << Time::nosecond << time;
    EXPECT_EQ(stream.str(), "333");
    
    // Back to default
    reset_stream();
    stream << Time::nomillisecond << time;
    EXPECT_EQ(stream.str(), "09:24:05.333");
}

TEST(Time, format_12hr)
{
    Time time(20, 42, 12, 5);
    std::stringstream stream;
    auto reset_stream = [&stream] () {
        stream.str(std::string());
    };
    
    stream << Time::am_pm;
    
    stream << time;
    EXPECT_EQ(stream.str(), "8:42:12.005 PM");
    
    time = Time(12, 42, 12, 5);
    reset_stream();
    stream << time;
    EXPECT_EQ(stream.str(), "12:42:12.005 PM");
    
    time = Time(13, 42, 12, 5);
    reset_stream();
    stream << time;
    EXPECT_EQ(stream.str(), "1:42:12.005 PM");
    
    time = Time(0, 42, 12, 5);
    reset_stream();
    stream << time;
    EXPECT_EQ(stream.str(), "12:42:12.005 AM");
}

TEST(Time, input_operator)
{
    Time time(20, 42, 12, 5);
    std::stringstream stream("invalid_time");
    
    auto reset_stream = [&stream] () {
        stream.str(std::string());
        stream.clear();
    };
    
    Time read_time;
    stream >> read_time;
    EXPECT_TRUE(stream.fail());
    
    reset_stream();
    
    stream << time << "garbage";
    stream >> read_time;
    
    EXPECT_FALSE(stream.fail());
    EXPECT_EQ(read_time, time);
    
    reset_stream();
    
    stream << Time::nohour << Time::minute << Time::second << Time::millisecond << time;
    
    read_time = Time();
    stream >> read_time;
    EXPECT_EQ(read_time, Time(0, 42, 12, 5));
    EXPECT_FALSE(stream.fail());
}
