//
// Created by Dániel Molnár on 2019-03-03.
//

#include <gtest/gtest.h>
#include <TestUtil.hpp>
#include <thread>
#include <chrono>

#include "ThreadPool.hpp"

using namespace Core;

TEST(ThreadPool, canBeConstructed)
{
    using namespace std::chrono_literals;
    ThreadPool<10> pool;
}
