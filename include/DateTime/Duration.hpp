//
// Created by Dániel Molnár on 2019-04-15.
//

#pragma once
#ifndef CORE_DURATION_HPP
#define CORE_DURATION_HPP

#include <chrono>

namespace Core {
    template<class Rep, class Period = std::ratio<1>>
    class Duration : public std::chrono::duration<Rep, Period> {
    private:
        using Base = std::chrono::duration<Rep, Period>;
    public:
        constexpr Duration() = default;
        
        Duration(const std::chrono::duration<Rep, Period>& other) : Base(other) {}
        
        template<class Rep2>
        constexpr explicit Duration(const Rep2& t) : Base(t) {}
        
        template<class Rep2, class Period2>
        constexpr Duration(const std::chrono::duration<Rep2, Period2>& other) : Base(other) {}
    };
}

#endif //CORE_DURATION_HPP
