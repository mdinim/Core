//
// Created by Dániel Molnár on 2019-03-03.
//

#include <gtest/gtest.h>
#include <Utils/TestUtil.hpp>
#include <thread>
#include <chrono>

#include <ThreadPool/ThreadPool.hpp>

using namespace Core;

class Functor
{
public:
    double operator()(int, std::string)
    {
        return .012;
    }
};

std::string foo() {
    return "Hello World!";
}

TEST(ThreadPool, canBeConstructed)
{
    ThreadPool<10> pool;
    ASSERT_FALSE(pool.has_queued_job());
}

TEST(ThreadPool, resultCanBeRetrieved)
{
    ThreadPool<5> pool;

    auto intResult = pool.add_job([]() { return 1; });
    auto stringResult = pool.add_job(foo);
    auto doubleResult = pool.add_job(Functor(), 1, "");

    ASSERT_EQ(intResult.get(), 1);
    ASSERT_EQ(stringResult.get(), "Hello World!");
    ASSERT_EQ(doubleResult.get(), .012);
}

TEST(ThreadPool, runsParalell)
{
    using namespace std::chrono_literals;
    auto sleep100ms = []() {
        std::this_thread::sleep_for(100ms);
    };

    ThreadPool<1> singleThreadPool;
    auto timeAtQueueing = std::chrono::high_resolution_clock::now();
    singleThreadPool.add_job(sleep100ms);
    singleThreadPool.add_job(sleep100ms);
    auto lastResult = singleThreadPool.add_job(sleep100ms);
    lastResult.get();
    auto timeAtEnd = std::chrono::high_resolution_clock::now();
    auto elapsedMilliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(timeAtEnd - timeAtQueueing).count();

    ASSERT_GE(elapsedMilliseconds, 300);

    ThreadPool<3> triThreadPool;
    timeAtQueueing = std::chrono::high_resolution_clock::now();
    triThreadPool.add_job(sleep100ms);
    triThreadPool.add_job(sleep100ms);
    triThreadPool.add_job(sleep100ms);
    lastResult = triThreadPool.add_job(sleep100ms);
    lastResult.get();
    timeAtEnd = std::chrono::high_resolution_clock::now();
    elapsedMilliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(timeAtEnd - timeAtQueueing).count();

    ASSERT_GE(elapsedMilliseconds, 200);
    ASSERT_LE(elapsedMilliseconds, 220);
}

TEST(ThreadPool, hasQueuedJobFlag)
{
    using namespace std::chrono_literals;
    ThreadPool<1> pool;
    ASSERT_FALSE(pool.has_queued_job());

    pool.add_job([]() { std::this_thread::sleep_for(100ms); });
    auto sleepForAWhile = pool.add_job([]() {
        std::this_thread::sleep_for(100ms);
        return 2;
    });

    ASSERT_TRUE(pool.has_queued_job());
    sleepForAWhile.get();
    ASSERT_FALSE(pool.has_queued_job());
}

TEST(ThreadPool, gracefulStop)
{
    using namespace std::chrono_literals;

    std::vector<std::future<unsigned>> futures;
    {
        ThreadPool<5> pool;

        futures.reserve(1000);
        for (auto i = 0u; i < 1000u; ++i) {
            futures.push_back(pool.add_job([i]() {
                std::this_thread::sleep_for(1ms);
                return i;
            }));
        }

        pool.stop(true);
        auto invalidFuture = pool.add_job([]() { return -1; });
        ASSERT_FALSE(invalidFuture.valid());
        ASSERT_TRUE(pool.has_queued_job());
    }

    for (auto i = 0u; i < 1000u; ++i) {
        auto& future = futures.at(i);
        auto result = future.get();
        ASSERT_EQ(result, i);
    }
}
