//
// Created by Dániel Molnár on 2019-04-14.
//

#pragma once
#ifndef CORE_BASELOGGER_HPP
#define CORE_BASELOGGER_HPP

#include <chrono>
#include <string>

#include <DateTime/Time.hpp>

#include <Logger/ILogger.hpp>

namespace Core {
/// \brief Implementation of general ideas.
class BaseLogger : public ILogger {
  protected:
    /// \brief Represents a single log entry (combination of timestamp and
    /// message)
    struct LogEntry {
        LogEntry() = default;

        /// \brief Constructs a single log entry. Timestamp is extracted from
        /// system time.
        LogEntry(std::string message_) : message(std::move(message_)) {
            auto now = std::chrono::system_clock::now();
            auto epoch = std::chrono::time_point<std::chrono::system_clock>();

            auto diff = now - epoch;

            timestamp = Time(
                std::chrono::duration_cast<std::chrono::hours>(diff).count() %
                    24,
                std::chrono::duration_cast<std::chrono::minutes>(diff).count() %
                    60,
                std::chrono::duration_cast<std::chrono::seconds>(diff).count() %
                    60,
                std::chrono::duration_cast<std::chrono::milliseconds>(diff)
                        .count() %
                    1000);
        }

        /// \brief Holds the timestamp.
        Time timestamp;

        /// \brief Log message.
        std::string message;

        /// \brief Output operator. Prints the log entry to the output stream.
        friend std::ostream &operator<<(std::ostream &os,
                                        const LogEntry &entry) {
            os << "[" << entry.timestamp << "] - " << entry.message;
            return os;
        }
    };

  public:
    /// \copydoc ILogger::info
    void info(const std::string &message) override { log(INFO_LEVEL, message); }

    /// \copydoc ILogger::warning
    void warning(const std::string &message) override {
        log(WARNING_LEVEL, message);
    }

    /// \copydoc ILogger::error
    void error(const std::string &message) override {
        log(ERROR_LEVEL, message);
    }
};
} // namespace Core

#endif // CORE_BASELOGGER_HPP
