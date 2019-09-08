//
// Created by Dániel Molnár on 2019-04-15.
//

#include <DateTime/Time.hpp>

#include <functional>
#include <array>
#include <algorithm>
#include <sstream>

namespace Core {
    const int Time::ios_format_index = std::ios_base::xalloc();
    const int Time::ios_element_mask_index = std::ios_base::xalloc();
    
    Time::Time(unsigned int hour, unsigned int minute, unsigned int second, unsigned int millisecond) {
        _seconds_since_epoch = millisecond / 1000.0;
        _seconds_since_epoch += second;
        _seconds_since_epoch += minute * 60;
        _seconds_since_epoch += hour * 60 * 60;
        
        normalize();
    }
    
    bool Time::operator==(const Time& rhs) const {
        return std::abs(_seconds_since_epoch - rhs._seconds_since_epoch) < 0.001;
    }
    bool Time::operator!=(const Time& rhs) const {
        return !(*this == rhs);
    }
    
    std::ostream& operator<<(std::ostream& os, const Time& time) {
        using TimeFormat = Time::TimeFormat;
        using TimeElement = Time::TimeElement;
        
        auto current_time_format = static_cast<TimeFormat>(os.iword(Time::ios_format_index));
        std::ios init(nullptr);
        init.copyfmt(os);
        
        bool reset_mask = false;
        if (os.iword(Time::ios_element_mask_index) == 0) { // No mask present
            os << Time::default_mask;
            reset_mask = true;
        }
        
        os << std::setfill('0');
        
        const bool is_am_pm = current_time_format == TimeFormat::AmPm;
        const std::array<const unsigned int, 4> elements =
            { time.hour(), time.minute(), time.second(), time.millisecond() };
        const std::array<short, 4> widths = { static_cast<short>((is_am_pm ? 1 : 2)), 2, 2, 3 };
        
        static_assert(elements.size() == Time::element_masks.size());
        
        bool data_written = false;
        
        for (auto i = 0u; i < elements.size(); ++i) {
            const auto& element_mask = Time::element_masks.at(i);
            if (Time::is_element_on(os, element_mask)) {
                const auto& element = elements.at(i);
                if (data_written) {
                    os << Time::separators.at(i);
                } else {
                    data_written = true;
                }
                
                if (current_time_format == TimeFormat::AmPm && element_mask == TimeElement::Hour) {
                    os << std::setw(widths.at(i)) << ((element % 12) == 0 ? 12 : (element % 12));
                } else {
                    os << std::setw(widths.at(i)) << element;
                }
            }
        }
        
        if (is_am_pm && Time::is_element_on(os, TimeElement::Hour)) {
            os << " " << (time.hour() >= 12 ? "PM" : "AM");
        }
        
        if (reset_mask) {
            os.iword(Time::ios_element_mask_index) = 0;
        }
        
        os.copyfmt(init);
        
        return os;
    }

    std::istream& operator>>(std::istream& is, Time& time) {
        using TimeElement = Time::TimeElement;
        
        bool reset_mask = false;
        time._seconds_since_epoch = 0;
        
        if (is.iword(Time::ios_element_mask_index) == 0) {
            is >> Time::default_mask;
            reset_mask = true;
        }
        
        auto i = 0u;
        for (const auto& element_mask : Time::element_masks) {
            auto& separator = Time::separators.at(std::min<std::size_t>(i++ + 1, Time::separators.size() - 1));
            
            if (Time::is_element_on(is, element_mask)) {
                std::string string;
                unsigned int element;
                std::getline(is, string, separator);
                std::stringstream stream(string);
                
                stream >> element;
                
                if (stream.fail()) {
                    is.setstate(std::ios::failbit);
                }
                
                switch(element_mask) {
                    case TimeElement::Hour:
                        time._seconds_since_epoch += element * 60 * 60;
                        break;
                    case TimeElement::Minute:
                        time._seconds_since_epoch += element * 60;
                        break;
                    case TimeElement::Second:
                        time._seconds_since_epoch += element;
                        break;
                    case TimeElement::Millisecond:
                        time._seconds_since_epoch += (element / 1000.0);
                        break;
                }
            }
        }
        
        if (reset_mask) {
            is.iword(Time::ios_element_mask_index) = 0;
        }
        
        time.normalize();
        
        return is;
    }
}
