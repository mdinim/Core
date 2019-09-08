//
// Created by Dániel Molnár on 2019-09-08.
//

#include <Logger/StreamLogger.hpp>
#include <gtest/gtest.h>

using namespace Core;
TEST(Logger, construction) {
    using namespace std::chrono_literals;
    std::stringstream stream;
    StreamLogger logger(100, stream);
    for (int i = 0; i < 5; ++i) {
        std::thread([&logger, i]() {
            std::this_thread::sleep_for(10ms);
            logger.info(std::to_string(i));
        }).detach();
    }
    std::this_thread::sleep_for(50ms);

    std::string str;
    int i = 0;
    while (std::getline(stream, str)) {
        ++i;
    }

    ASSERT_EQ(i, 5);
}

TEST(Logger, priority) {
    std::stringstream stream;
    { // Scope to ensure logger has emptied its queue.
        StreamLogger logger(WARNING_LEVEL, stream);
        logger.warning("stuff");
        logger.info("stuffity");
        logger.error("what");
    }
    std::string str;
    int i = 0;
    while (std::getline(stream, str)) {
        ++i;
    }

    ASSERT_EQ(i, 2);
}