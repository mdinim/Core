//
// Created by Dániel Molnár on 2019-08-16.
//

#include <gtest/gtest.h>

#include <Utils/TestUtil.hpp>
#include <Utils/Utils.hpp>
#include <Utils/Size.hpp>

#include <vector>

using namespace Core;

TEST(UtilsTest, is_container) {
    struct dummy {};
    class vector_like : public std::vector<int> {};
    ASSERT_TRUE(is_container<std::vector<int>>::value);
    bool is_map_container = is_container<std::map<int, int>>::value;
    ASSERT_TRUE(is_map_container);
    ASSERT_TRUE(is_container<vector_like>::value);
    ASSERT_FALSE(is_container<int>::value);
    ASSERT_FALSE(is_container<dummy>::value);
}

TEST(UtilsTest, size_literals)
{
    using namespace SizeLiterals;

    ASSERT_EQ(1024_KB, 1.0_MB);
    ASSERT_EQ(1536_KB, 1.5_MB);
    ASSERT_EQ(1_KB, 8192_b);
    ASSERT_EQ(1_KB + 15_MB, 15729664_B);
    auto mb = 4_MB;
    mb += 1_MB;
    ASSERT_EQ(5_MB, mb);
    mb -= 2_MB;
    ASSERT_EQ(3_MB, mb);
    mb = -12_MB;
    ASSERT_EQ(-12, mb.value);
    auto copy = mb++;
    ASSERT_EQ(copy + 1_MB, mb);
    auto copy_two = ++mb;
    ASSERT_EQ(copy_two, mb);
    ASSERT_EQ(--1_MB, 0_b);
    ASSERT_EQ(1_MB--, 1024_KB);
    ASSERT_EQ(1_MB - 1024_KB, 0_b);

    std::stringstream stream;
    auto reset_stream = [&stream] () {
      stream.str(std::string());
    };
    stream << 1_MB;
    ASSERT_EQ(stream.str(), "1 MB");
    reset_stream();
    stream << 1124_KB;
    ASSERT_EQ(stream.str(), "1124 KB");
    reset_stream();
    stream << 358_B;
    ASSERT_EQ(stream.str(), "358 B");
    reset_stream();
    stream << -124124358_b;
    ASSERT_EQ(stream.str(), "-124124358 b");
    reset_stream();
    stream << -25_GB;
    ASSERT_EQ(stream.str(), "-25 GB");

    ASSERT_LT(2_GB, 4096_MB);
    ASSERT_GT(2_GB + 2049_MB, 4096_MB);

    ASSERT_GE(2_GB + 2049_MB, 4096_MB + 1024_KB);

    ASSERT_EQ(2_GB + 2, 4098_MB - 2);

    auto four_gigs = 4_GB;
    four_gigs -= 2;
    ASSERT_EQ(four_gigs, 4096_MB - 2_GB);

    ASSERT_LE(four_gigs, 4096_MB);

    SizeLiterals::Byte one_kb = 1_KB;
    ASSERT_EQ(1024_B, one_kb);

    ASSERT_EQ((2_GB / 2), 1024_MB);

    ASSERT_EQ(1_GB / 2, 512_MB);
}
