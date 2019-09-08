//
// Created by Dániel Molnár on 2019-09-08.
//

#include <optional>

#include <Logger/StreamLogger.hpp>

namespace Core {
StreamLogger::StreamLogger(unsigned int enabled_level, std::ostream &stream)
    : _enabled_level(enabled_level), _stream(stream) {
    _logger_thread = std::thread([this] {
        while (!_should_stop || !_entry_queue.empty()) {
            std::optional<LogEntry> top;
            {
                std::unique_lock lock(_queue_guard);
                _has_entry.wait(lock, [this]() {
                    return _should_stop || !_entry_queue.empty();
                });
                if (!_entry_queue.empty()) {
                    top = std::move(_entry_queue.front());
                    _entry_queue.pop();
                }
            }
            if (top)
                _stream << *top << std::endl;
        }
    });
}

StreamLogger::~StreamLogger() {
    _should_stop = true;
    _has_entry.notify_all();
    _logger_thread.join();
}

void StreamLogger::log(unsigned level, const std::string &message) {
    if (level <= _enabled_level) {
        std::unique_lock lock(_queue_guard);
        _entry_queue.emplace(message);
    }
    _has_entry.notify_all();
}
}