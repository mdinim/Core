//
// Created by Dániel Molnár on 2019-06-04.
//

#include <DateTime/Duration.hpp>

namespace Core {
    Duration::Duration(int years, int months, int days, int hours, int seconds, int milliseconds)
        : _years(years), _months(months), _days(days), _hours(hours), _seconds(seconds), _milliseconds(milliseconds)
    {
        
    }
}
