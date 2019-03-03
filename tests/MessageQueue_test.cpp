//
// Created by Dániel Molnár on 2019-03-03.
//

#include <MessageQueue.hpp>
#include <TestUtil.hpp>

#include <gtest/gtest.h>

#include <thread>
#include <chrono>

TEST(MessageQueue, canBeConstructed)
{
    MessageQueue<std::string> simpleQueue;
}

TEST(MessageQueue, canDispatchMessage)
{
    MessageQueue<std::string> simpleQueue;
    simpleQueue.push("Hey there");

    ASSERT_FALSE(simpleQueue.empty());
}

TEST(MessageQueue, canRetreiveMessage)
{
    MessageQueue<std::string> simpleQueue;
    simpleQueue.push("Hey there");

    ASSERT_FALSE(simpleQueue.empty());
    ASSERT_EQ(simpleQueue.take(), "Hey there");
    ASSERT_TRUE(simpleQueue.empty());
}

TEST(MessageQueue, priorityMatters)
{
    MessageQueue<std::string> simpleQueue;
    simpleQueue.push(MessagePriority::Low, "!");
    simpleQueue.push(MessagePriority::High, "Hello");
    simpleQueue.push("world");
    simpleQueue.push(static_cast<unsigned>(MessagePriority::Normal) + 1, " ");

    std::string result = "";
    while(!simpleQueue.empty())
    {
        result += simpleQueue.take();
    }
    ASSERT_EQ(result, "Hello world!");
}

TEST(MessageQueue, messagesCanBeWaitedFor)
{
    using namespace std::chrono_literals;

    MessageQueue<std::string> simpleQueue;
    std::thread dispatcherThreadOne([&simpleQueue](){
        std::this_thread::sleep_for(.5s);
        simpleQueue.push("delayed msg one");
    });
    std::thread dispatcherThreadTwo([&simpleQueue](){
        std::this_thread::sleep_for(1s);
        simpleQueue.push("delayed msg two");
    });

    auto timeAtWait = std::chrono::high_resolution_clock::now();
    simpleQueue.waitForMessage();
    simpleQueue.take();
    auto timeAtWaitEnd = std::chrono::high_resolution_clock::now();
    auto millisecElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timeAtWaitEnd - timeAtWait).count();

    TEST_INFO << "Elapsed time #1: " << millisecElapsed << " ms" << std::endl;
    ASSERT_GE(millisecElapsed, 500);

    simpleQueue.waitForMessage();
    timeAtWaitEnd = std::chrono::high_resolution_clock::now();
    millisecElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timeAtWaitEnd - timeAtWait).count();

    TEST_INFO << "Elapsed time #2: " << millisecElapsed << " ms" << std::endl;
    ASSERT_GE(millisecElapsed, 1000);
    dispatcherThreadOne.join();
    dispatcherThreadTwo.join();
}
