//
// Created by Dániel Molnár on 2019-04-15.
//

#pragma once
#ifndef CORE_TIME_HPP
#define CORE_TIME_HPP

#include <iostream>
#include <iomanip>
#include <chrono>
#include <array>
#include <ratio>
#include <cmath>

#include <DateTime/Duration.hpp>

namespace Core {
    /// \brief Representing the concept of Time only.
    /// Precision up to milliseconds.
    class Time {
    private:
        /// \brief Format of time. Can be used with io streams.
        enum class TimeFormat {
            Iso = 0,
            AmPm
        };
        
        /// \brief Elements of time. Can be used with io streams.
        enum class TimeElement {
            Hour = 1 << 0,
            Minute = 1 << 1,
            Second = 1 << 2,
            Millisecond = 1 << 3,
        };
        
        /// \brief Allocated index on stream for the current format.
        static const int ios_format_index;
        
        /// \brief Allocated index on stream for the element mask.
        static const int ios_element_mask_index;
        
        double _seconds_since_epoch = 0;

        /// \brief Set the format on \a ios to \a format.
        static void set_ios_format(std::ios& ios, TimeFormat format) {
            ios.iword(ios_format_index) = static_cast<int>(format);
        }
        
        /// \brief Add the element \a element to the stream \a ios.
        static void add_ios_element(std::ios& ios, TimeElement element) {
            ios.iword(ios_element_mask_index) |= static_cast<int>(element);
        }
        
        /// \brief Remove the element \a element from the stream \a ios.
        static void remove_ios_element(std::ios& ios, TimeElement element) {
             ios.iword(ios_element_mask_index) &= ~(static_cast<int>(element));
        }
        
        /// \brief Check if the element \a element is currently enabled on \a ios.
        static bool is_element_on(std::ios& ios, TimeElement element ) {
            return ios.iword(ios_element_mask_index) & static_cast<int>(element);
        }
        
        /// \brief Container for elements.
        static constexpr std::array<TimeElement, 4> element_masks
            { TimeElement::Hour, TimeElement::Minute, TimeElement::Second, TimeElement::Millisecond };
        
        /// \brief Container of separators.
        static constexpr const std::array<char, 4> separators
            { ':', ':', ':', '.'};
        
        void normalize() {
            unsigned whole_part = static_cast<unsigned>(_seconds_since_epoch) % (24 * 60 * 60);
            double fractional_part = _seconds_since_epoch - static_cast<unsigned>(_seconds_since_epoch);
            
            _seconds_since_epoch = whole_part + fractional_part;
        }
    public:
        /// \brief Default constructor (constructs 00:00:00.0)
        Time() = default;
        
        /// \brief Construct a time point.
        Time(unsigned int hour, unsigned int minute = 0, unsigned int second = 0, unsigned int millisecond = 0);

        /// \brief Get millisecond.
        unsigned short millisecond() const {
            return static_cast<unsigned long>(_seconds_since_epoch * 1000) % 1000;
        }

        /// \brief Get second.
        unsigned short second() const {
            return static_cast<unsigned long>(_seconds_since_epoch) % 60;
        }
        
        /// \brief Get minute.
        unsigned short minute() const {
            return static_cast<unsigned long>(_seconds_since_epoch) / 60 % 60;
        }

        /// \brief Get hour.
        unsigned short hour() const {
            return static_cast<unsigned long>(_seconds_since_epoch / 60 / 60) % 24;
        }

        /// \brief Equality operator.
        bool operator==(const Time&) const;
        
        /// \brief Inequality operator.
        bool operator!=(const Time&) const;
        
        /// \brief Add \a duration to the time point.
        template<class Rep, class Period>
        void operator+=(const std::chrono::duration<Rep, Period>& duration) {
            _seconds_since_epoch += std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
            
            normalize();
        }
        
        /// \brief Subtract \a duration to the time point.
        template<class Rep, class Period>
        void operator-=(const std::chrono::duration<Rep, Period>& duration) {
            *this += -duration;
        }
        
        /// Construct a time later/earlier than this.
        template<class Rep, class Period>
        Time operator-(const std::chrono::duration<Rep, Period>& duration) const {
            Time result = *this;
            result -= duration;
            return result;
        }
        
        /// Construct a time later/earlier than this.
        template<class Rep, class Period>
        Time operator+(const std::chrono::duration<Rep, Period>& duration) const {
            Time result = *this;
            result += duration;
            return result;
        }

        template<class Rep = double, class Period = std::ratio<1>>
        typename std::common_type<Duration<Rep, Period>, Duration<Rep>>::type operator-(const Time& rhs) const {
            using ResultType = typename std::common_type<Duration<Rep, Period>, Duration<Rep>>::type;
            auto difference = static_cast<Rep>(_seconds_since_epoch - rhs._seconds_since_epoch);
            
            return ResultType(difference);
        }
        
        /// \brief Stream manipulator for format ISO
        static std::ios& iso(std::ios& ios) {
            set_ios_format(ios, TimeFormat::Iso);
            return ios;
        }
        
        /// \brief Stream manipulator for format AM/PM.
        static std::ios& am_pm(std::ios& ios) {
            set_ios_format(ios, TimeFormat::AmPm);
            return ios;
        }
        
        /// \brief Stream manipulator for enabling Hour element.
        static std::ios& hour(std::ios& ios) {
            add_ios_element(ios, TimeElement::Hour);
            return ios;
        }
        
        /// \brief Stream manipulator for disabling Hour element.
        static std::ios& nohour(std::ios& ios) {
            remove_ios_element(ios, TimeElement::Hour);
            return ios;
        }
        
        /// \brief Stream manipulator for enabling Minute element.
        static std::ios& minute(std::ios& ios) {
            add_ios_element(ios, TimeElement::Minute);
            return ios;
        }
        
        /// \brief Stream manipulator for disabling Minute element.
        static std::ios& nominute(std::ios& ios) {
            remove_ios_element(ios, TimeElement::Minute);
            return ios;
        }
        
        /// \brief Stream manipulator for enabling Second element.
        static std::ios& second(std::ios& ios) {
            add_ios_element(ios, TimeElement::Second);
            return ios;
        }
        
        /// \brief Stream manipulator for disabling Second element.
        static std::ios& nosecond(std::ios& ios) {
            remove_ios_element(ios, TimeElement::Second);
            return ios;
        }
        
        /// \brief Stream manipulator for enabling Millisecond element.
        static std::ios& millisecond(std::ios& ios) {
            add_ios_element(ios, TimeElement::Millisecond);
            return ios;
        }
        
        /// \brief Stream manipulator for disabling Millisecond element.
        static std::ios& nomillisecond(std::ios& ios) {
            remove_ios_element(ios, TimeElement::Millisecond);
            return ios;
        }
        
        /// \brief Enable default elements (all)
        static std::ios& default_mask(std::ios& ios) {
            Time::hour(ios);
            Time::minute(ios);
            Time::second(ios);
            Time::millisecond(ios);
            return ios;
        }
        
        /// \brief Print time to stream \a os.
        friend std::ostream& operator<<(std::ostream& os, const Time& time);
        
        /// \brief Read time from stream \a is.
        friend std::istream& operator>>(std::istream& is, Time& time);
    };
}

#endif //CORE_TIME_HPP
