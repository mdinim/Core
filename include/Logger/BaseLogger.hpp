//
// Created by Dániel Molnár on 2019-04-14.
//

#pragma once
#ifndef CORE_BASELOGGER_HPP
#define CORE_BASELOGGER_HPP

#include <Logger/ILogger.hpp>

namespace Core {
    class BaseLogger : public ILogger {
        class LogEntry {
        };

        void info(const std::string& message) override {
            log(INFO_LEVEL, message);
        }

        void warning(const std::string& message) override {
            log(WARNING_LEVEL, message);
        }

        void error(const std::string& message) override {
            log(ERROR_LEVEL, message);
        }
    };
}

#endif //CORE_BASELOGGER_HPP
