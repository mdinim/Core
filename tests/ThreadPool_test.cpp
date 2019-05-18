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

auto sleep_for = [](std::chrono::milliseconds dur) {
    return [dur]() {
        std::this_thread::sleep_for(dur);
    };
};

TEST(ThreadPool, can_be_constructed)
{
    ThreadPool<10> pool;
    ASSERT_FALSE(pool.has_queued_job());
}

TEST(ThreadPool, result_can_be_retrieved)
{
    ThreadPool<5> pool;

    auto int_result = pool.add_job([]() { return 1; });
    auto string_result = pool.add_job(foo);
    auto double_result = pool.add_job(Functor(), 1, "");

    ASSERT_EQ(int_result.get(), 1);
    ASSERT_EQ(string_result.get(), "Hello World!");
    ASSERT_EQ(double_result.get(), .012);
}

TEST(ThreadPool, runs_parallel)
{
    using namespace std::chrono_literals;
    auto sleep_100ms = sleep_for(100ms);

    ThreadPool<1> single_thread_pool;
    auto time_at_start = std::chrono::high_resolution_clock::now();
    single_thread_pool.add_job(sleep_100ms);
    single_thread_pool.add_job(sleep_100ms);
    auto last_result = single_thread_pool.add_job(sleep_100ms);
    last_result.get();
    auto time_at_end = std::chrono::high_resolution_clock::now();
    auto elapsed_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(time_at_end - time_at_start).count();

    ASSERT_GE(elapsed_ms, 300);

    ThreadPool<3> tri_thread_pool;
    time_at_start = std::chrono::high_resolution_clock::now();
    tri_thread_pool.add_job(sleep_100ms);
    tri_thread_pool.add_job(sleep_100ms);
    tri_thread_pool.add_job(sleep_100ms);
    last_result = tri_thread_pool.add_job(JobPriority::Low, sleep_100ms);
    last_result.wait();
    time_at_end = std::chrono::high_resolution_clock::now();
    elapsed_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(time_at_end - time_at_start).count();

    ASSERT_GE(elapsed_ms, 200);
    ASSERT_LE(elapsed_ms, 220);
}

TEST(ThreadPool, has_queued_job_flag)
{
    using namespace std::chrono_literals;
    ThreadPool<1> pool;
    ASSERT_FALSE(pool.has_queued_job());

    pool.add_job(sleep_for(100ms));
    auto slept_future = pool.add_job([]() {
        std::this_thread::sleep_for(100ms);
        return 2;
    });

    ASSERT_TRUE(pool.has_queued_job());
    slept_future.get();
    ASSERT_FALSE(pool.has_queued_job());
}

TEST(ThreadPool, prioritized_tasks)
{
    using namespace std::chrono_literals;

    ThreadPool<1> pool;

    std::vector<unsigned> results;
    auto first_job = pool.add_job(sleep_for(50ms));

    unsigned priority = static_cast<unsigned>(JobPriority::Low);
    auto get_prio_job = [&results, &priority]() {
        return  [&results, priority]() {
            results.push_back(priority);
        };
    };

    // First job takes 50 ms, pour a bunch of prioritized jobs in.
    auto low_job = pool.add_job(priority, get_prio_job());
    std::vector<std::future<void>> futures;

    priority = static_cast<unsigned>(JobPriority::High);
    futures.push_back(pool.add_job(priority, get_prio_job()));

    priority = static_cast<unsigned>(JobPriority::Low);
    futures.push_back(pool.add_job(priority, get_prio_job()));

    priority = static_cast<unsigned>(JobPriority::High) + 1;
    futures.push_back(pool.add_job(priority, get_prio_job()));

    priority = static_cast<unsigned>(JobPriority::Normal) + 1;
    futures.push_back(pool.add_job(priority, get_prio_job()));

    // wait for the rest to start
    first_job.wait();

    for(const auto& future : futures) {
        future.wait();
    }
    ASSERT_FALSE(pool.has_queued_job());

    // if all went well, the jobs ran in order of their priority, not in the order they were queued in
    // the results vector should be sorted backwards.
    ASSERT_TRUE(std::is_sorted(results.begin(), results.end(), std::greater<>()));
}

TEST(ThreadPool, graceful_stop)
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
