//
// Created by Dániel Molnár on 2019-03-03.
//

#include <MessageQueue/MessageQueue.hpp>
#include <Utils/TestUtil.hpp>

#include <gtest/gtest.h>

#include <thread>
#include <chrono>

using namespace Core;

TEST(MessageQueue, can_be_constructed)
{
    MessageQueue<std::string> simpleQueue;
}

TEST(MessageQueue, can_dispatch_message)
{
    MessageQueue<std::string> simple_queue;
    simple_queue.push("Hey there");

    ASSERT_FALSE(simple_queue.empty());
}

TEST(MessageQueue, can_retrieve_message)
{
    MessageQueue<std::string> simple_queue;
    simple_queue.push("Hey there");

    ASSERT_FALSE(simple_queue.empty());
    ASSERT_EQ(simple_queue.take(), "Hey there");
    ASSERT_TRUE(simple_queue.empty());
}

TEST(MessageQueue, priority_matters)
{
    MessageQueue<std::string> simple_queue;
    simple_queue.push(MessagePriority::Low, "!");
    simple_queue.push(MessagePriority::High, "Hello");
    simple_queue.push("world");
    simple_queue.push(static_cast<unsigned>(MessagePriority::Normal) + 1, " ");

    std::string result;
    while(!simple_queue.empty())
    {
        result += simple_queue.take();
    }
    ASSERT_EQ(result, "Hello world!");
}

TEST(MessageQueue, messages_can_be_waited_for)
{
    using namespace std::chrono_literals;

    MessageQueue<std::string> simple_queue;
    std::thread thread_one([&simple_queue]() {
        std::this_thread::sleep_for(.5s);
        simple_queue.push("delayed msg one");
    });
    std::thread thread_two([&simple_queue]() {
        std::this_thread::sleep_for(1s);
        simple_queue.push("delayed msg two");
    });

    auto time_at_start = std::chrono::high_resolution_clock::now();
    simple_queue.wait_for_message();
    simple_queue.take();
    auto time_at_end = std::chrono::high_resolution_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_at_end - time_at_start).count();

    TEST_INFO << "Elapsed time #1: " << elapsed_ms << " ms" << std::endl;
    ASSERT_GE(elapsed_ms, 500);

    simple_queue.wait_for_message();
    time_at_end = std::chrono::high_resolution_clock::now();
    elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_at_end - time_at_start).count();

    TEST_INFO << "Elapsed time #2: " << elapsed_ms << " ms" << std::endl;
    ASSERT_GE(elapsed_ms, 1000);
    thread_one.join();
    thread_two.join();
}
