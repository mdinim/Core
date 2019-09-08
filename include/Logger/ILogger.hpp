//
// Created by Dániel Molnár on 2019-04-14.
//

#pragma once
#ifndef CORE_ILOGGER_HPP
#define CORE_ILOGGER_HPP

#include <string>

namespace Core {
    constexpr const unsigned INFO_LEVEL = 3;
    constexpr const unsigned WARNING_LEVEL = 2;
    constexpr const unsigned ERROR_LEVEL = 1;

    class ILogger {
    public:
        /// \brief Log a single message with a specific priority.
        virtual void log(unsigned, const std::string&) = 0;

        /// \brief Log a message with the built-in INFO_LEVEL priority
        virtual void info(const std::string&) = 0;

        /// \brief Log a message with the built-in WARNING_LEVEL priority
        virtual void warning(const std::string&) = 0;

        /// \brief Log a message with the built-in ERROR_LEVEL priority
        virtual void error(const std::string&) = 0;
    };
}

#endif //CORE_ILOGGER_HPP
