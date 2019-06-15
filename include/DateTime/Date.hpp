//
// Created by Dániel Molnár on 2019-04-15.
//

#pragma once
#ifndef CORE_DATE_HPP
#define CORE_DATE_HPP

namespace Core {
    class Date {
    public:
        int day() const;

        int month() const;

        int year() const;
    private:
        int _day;

        int _month;
        
        int _year;
    };
}

#endif //CORE_DATE_HPP
