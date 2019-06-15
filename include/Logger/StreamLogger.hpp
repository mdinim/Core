//
// Created by Dániel Molnár on 2019-04-14.
//

#ifndef CORE_STREAMLOGGER_HPP
#define CORE_STREAMLOGGER_HPP

#include <Logger/BaseLogger.hpp>

namespace Core {
    class StreamLogger : public BaseLogger {
    public:
        StreamLogger(const std::ostream&);

        void log(unsigned level, const std::string& message) override;
    };
}

#endif //CORE_STREAMLOGGER_HPP
