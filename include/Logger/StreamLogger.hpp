//
// Created by Dániel Molnár on 2019-04-14.
//

#ifndef CORE_STREAMLOGGER_HPP
#define CORE_STREAMLOGGER_HPP

#include <Logger/BaseLogger.hpp>

#include <atomic>
#include <mutex>
#include <ostream>
#include <queue>
#include <thread>
#include <condition_variable>

namespace Core {
/// \brief Logger that operates on a std::ostream object.
class StreamLogger : public BaseLogger {
  private:
    /// \brief Flag that indicates that the logger should stop its logging
    /// thread.
    std::atomic_bool _should_stop = false;

    /// \brief Indicates the level that the log level operates on. Any message
    /// above this level won't make it to the output stream.
    unsigned int _enabled_level;

    /// \brief Holds a reference to the stream to put messages on.
    std::ostream &_stream;

    /// \brief Guards the message queue.
    std::mutex _queue_guard;

    /// \brief Queue that holds the messages.
    std::queue<BaseLogger::LogEntry> _entry_queue;

    /// \brief StreamLogger separates its logging to a separate thread.
    std::thread _logger_thread;

    /// \brief Signals that the logger has a new entry or that the logger should
    /// stop.
    std::condition_variable _has_entry;

  public:
    /// \brief Constructs the logger.
    /// \param enabled_level Any log entry above this level won't make it to the
    /// logger target.
    StreamLogger(unsigned int enabled_level, std::ostream &);

    /// \brief Destructs the logger. Any log entry in the queue is going to be
    /// logged.
    ~StreamLogger();

    /// \brief Log a message with the specified level to the log target.
    void log(unsigned level, const std::string &message) override;
};
} // namespace Core

#endif // CORE_STREAMLOGGER_HPP
