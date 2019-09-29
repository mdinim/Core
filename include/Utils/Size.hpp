//
// Created by Dániel Molnár on 2019-08-16.
//

#ifndef CORE_SIZE_HPP
#define CORE_SIZE_HPP

#include <numeric>
#include <ostream>
#include <ratio>
#include <type_traits>

#include <Utils/Utils.hpp>

namespace Core {
namespace SizeLiterals {
/// \brief Alias for convenience
using b_ratio = std::ratio<1, 8>;

/// \brief Alias for convenience
using B_ratio = std::ratio<1>;

/// \brief Alias for convenience
using KB_ratio = std::ratio<B_ratio::num * 1024, 1>;

/// \brief Alias for convenience
using MB_ratio = std::ratio<KB_ratio::num * 1024, 1>;

/// \brief Alias for convenience
using GB_ratio = std::ratio<MB_ratio::num * 1024, 1>;

} // namespace SizeLiterals

/// \brief Represents the size of data and enables conversions between different
/// sizes
template <class Rep, class Ratio = std::ratio<1>> class Size {
  public:
    using RatioT = Ratio;
    static_assert(contains<Size::RatioT, SizeLiterals::b_ratio,
                           SizeLiterals::B_ratio, SizeLiterals::KB_ratio,
                           SizeLiterals::MB_ratio, SizeLiterals::GB_ratio>(),
                  "Unsupported ratio!");
    /// \brief Holds the amount in the current format
    Rep value;

    /// \brief Construction out of a value
    constexpr Size(Rep value_) : value(value_) {}

    /// \brief Copy construction (enables automatic conversion between different
    /// types)
    template <class Rep2, class Ratio2>
    constexpr Size(const Size<Rep2, Ratio2> &other) {
        value =
            other.value * Ratio2::num / Ratio2::den * Ratio::den / Ratio::num;
    }

    /// \brief Copy assignment (with automatic conversion)
    template <class Rep2, class Ratio2>
    constexpr Size<Rep, Ratio> &operator=(const Size<Rep2, Ratio2> &other) {
        value =
            other.value * Ratio2::num / Ratio2::den * Ratio::den / Ratio::num;
        return *this;
    }

    /// \brief Negate the underlying value (also enables construction with
    /// literals out of negative values)
    constexpr Size operator-() const { return -value; }

    /// \brief Pre-increment operator
    constexpr Size &operator++() {
        ++value;
        return *this;
    }

    /// \brief Pre-decrement operator
    constexpr Size &operator--() {
        --value;
        return *this;
    }

    /// \brief Post-increment operator
    constexpr Size operator++(int) {
        auto copy = *this;
        ++value;
        return copy;
    }

    /// \brief Post-decrement operator
    constexpr Size operator--(int) {
        auto copy = *this;
        --value;
        return copy;
    }

    /// /brief Subtract-assign operator
    constexpr Size &operator-=(const Rep &rhs) {
        (*this) += -rhs;
        return *this;
    }

    /// /brief Add-assign operator
    constexpr Size &operator+=(const Rep &rhs) {
        value += rhs;
        return *this;
    }

    /// /brief Add-assign operator
    template <class Rep2, class Ratio2>
    constexpr Size &operator+=(const Size<Rep2, Ratio2> &rhs) {
        Size converted_rhs = rhs;
        (*this) += converted_rhs.value;
        return *this;
    }

    /// /brief Subtract-assign operator
    template <class Rep2, class Ratio2>
    Size &operator-=(const Size<Rep2, Ratio2> &rhs) {
        (*this) += -rhs;
        return *this;
    }

    Size<int64_t, SizeLiterals::B_ratio> operator/(const Rep& divisor) const {
        Size<int64_t, SizeLiterals::B_ratio> ret = *this;
        return ret.value / divisor;
    }

    /// \brief output operator
    friend std::ostream &operator<<(std::ostream &os, const Size &size) {
        using namespace SizeLiterals;
        if constexpr (std::is_same_v<Size::RatioT, b_ratio>) {
            os << size.value << " b";
        }
        if constexpr (std::is_same_v<Size::RatioT, B_ratio>) {
            os << size.value << " B";
        }
        if constexpr (std::is_same_v<Size::RatioT, KB_ratio>) {
            os << size.value << " KB";
        }
        if constexpr (std::is_same_v<Size::RatioT, MB_ratio>) {
            os << size.value << " MB";
        }
        if constexpr (std::is_same_v<Size::RatioT, GB_ratio>) {
            os << size.value << " GB";
        }
        return os;
    }
}; // namespace Core


/// \brief Greater-than or equal operator
template <class Rep1, class Ratio1, class Rep2>
bool constexpr operator>=(const Size<Rep1, Ratio1> &lhs,
                          const Rep2 &rhs) {
    return lhs.value >= rhs;
}

/// \brief Less-than or equal operator
template <class Rep1, class Ratio1, class Rep2>
bool constexpr operator<=(const Size<Rep1, Ratio1> &lhs,
                          const Rep2 &rhs) {
    return lhs.value <= rhs;
}

/// \brief Equality operator
template <class Rep1, class Ratio1, class Rep2>
bool constexpr operator==(const Size<Rep1, Ratio1> &lhs,
                         const Rep2 &rhs) {
    return lhs.value == rhs;
}

/// \brief Greater-than operator
template <class Rep1, class Ratio1, class Rep2>
bool constexpr operator>(const Size<Rep1, Ratio1> &lhs,
                         const Rep2 &rhs) {
    return lhs.value > rhs;
}

/// \brief Less-than operator
template <class Rep1, class Ratio1, class Rep2>
bool constexpr operator<(const Size<Rep1, Ratio1> &lhs,
                         const Rep2 &rhs) {
    return lhs.value < rhs;
}


/// \brief Less-than operator
template <class Rep1, class Ratio1, class Rep2, class Ratio2>
bool constexpr operator<(const Size<Rep1, Ratio1> &lhs,
                         const Size<Rep2, Ratio2> &rhs) {
    using CommonSize =
        typename std::common_type<Size<Rep1, Ratio1>, Size<Rep2, Ratio2>>::type;
    CommonSize common_lhs = lhs;
    CommonSize common_rhs = rhs;

    return common_lhs.value < common_rhs.value;
}

/// \brief Greater-than operator
template <class Rep1, class Ratio1, class Rep2, class Ratio2>
bool constexpr operator>(const Size<Rep1, Ratio1> &lhs,
                         const Size<Rep2, Ratio2> &rhs) {
    return !(lhs < rhs) && lhs != rhs;
}

/// \brief Lesser-or-equal operator
template <class Rep1, class Ratio1, class Rep2, class Ratio2>
bool constexpr operator<=(const Size<Rep1, Ratio1> &lhs,
                          const Size<Rep2, Ratio2> &rhs) {
    return !(lhs > rhs);
}

/// \brief Greater-or-equal operator
template <class Rep1, class Ratio1, class Rep2, class Ratio2>
bool constexpr operator>=(const Size<Rep1, Ratio1> &lhs,
                          const Size<Rep2, Ratio2> &rhs) {
    return !(lhs < rhs);
}

/// \brief Inequality operator
template <class Rep1, class Ratio1, class Rep2, class Ratio2>
bool constexpr operator!=(const Size<Rep1, Ratio1> &lhs,
                          const Size<Rep2, Ratio2> &rhs) {
    return !(lhs == rhs);
}

/// \brief Equality operator
template <class Rep1, class Ratio1, class Rep2, class Ratio2>
bool constexpr operator==(const Size<Rep1, Ratio1> &lhs,
                          const Size<Rep2, Ratio2> &rhs) {
    using CommonSize =
        typename std::common_type<Size<Rep1, Ratio1>, Size<Rep2, Ratio2>>::type;
    CommonSize common_lhs = lhs;
    CommonSize common_rhs = rhs;
    return common_lhs.value == common_rhs.value;
}

/// \brief Add two sizes of different types (with automatic conversion)
template <class Rep1, class Ratio1, class Rep2, class Ratio2>
typename std::common_type<Size<Rep1, Ratio1>,
                          Size<Rep2, Ratio2>>::type constexpr
operator+(const Size<Rep1, Ratio1> &lhs, const Size<Rep2, Ratio2> &rhs) {
    using CommonSize =
        typename std::common_type<Size<Rep1, Ratio1>, Size<Rep2, Ratio2>>::type;
    CommonSize common_lhs = lhs;
    common_lhs += rhs;
    return common_lhs;
}

/// \brief Add two sizes of different types (with automatic conversion)
template <class Rep, class Ratio, class Rep2>
Size<Rep, Ratio> constexpr operator+(const Size<Rep, Ratio> &lhs,
                                     const Rep2 &rhs) {
    auto ret = lhs;
    ret += rhs;
    return ret;
}

/// \brief Add two sizes of different types (with automatic conversion)
template <class Rep, class Ratio, class Rep2>
Size<Rep, Ratio> constexpr operator-(const Size<Rep, Ratio> &lhs,
                                     const Rep2 &rhs) {
    return lhs + -rhs;
}

/// \brief Subtract two sizes of different types (utilizes the add operator)
template <class Rep1, class Ratio1, class Rep2, class Ratio2>
typename std::common_type<Size<Rep1, Ratio1>,
                          Size<Rep2, Ratio2>>::type constexpr
operator-(const Size<Rep1, Ratio1> &lhs, const Size<Rep2, Ratio2> &rhs) {
    return lhs + -rhs;
}

namespace SizeLiterals {
using Bit = Size<int64_t, b_ratio>;
using Byte = Size<int64_t, B_ratio>;
using KiloByte = Size<int64_t, KB_ratio>;
using MegaByte = Size<int32_t, MB_ratio>;
using GigaByte = Size<int16_t, GB_ratio>;

constexpr Bit operator""_b(unsigned long long int value) { return value; }

constexpr Byte operator""_B(unsigned long long int value) { return value; }

constexpr Size<double, B_ratio> operator""_B(long double value) {
    return value;
}

constexpr KiloByte operator""_KB(unsigned long long int value) { return value; }

constexpr Size<double, KB_ratio> operator""_KB(long double value) {
    return value;
}

constexpr MegaByte operator""_MB(unsigned long long int value) { return value; }

constexpr Size<double, MB_ratio> operator""_MB(long double value) {
    return value;
}

constexpr GigaByte operator""_GB(unsigned long long int value) {
    return GigaByte(value);
}

constexpr Size<double, GB_ratio> operator""_GB(long double value) {
    return value;
}
} // namespace SizeLiterals
} // namespace Core

namespace std {
template <class Rep1, class Ratio1, class Rep2, class Ratio2>
struct common_type<Core::Size<Rep1, Ratio1>, Core::Size<Rep2, Ratio2>> {
    typedef Core::Size<
        typename std::common_type<Rep1, Rep2>::type,
        std::ratio<std::gcd(Ratio1::num, Ratio2::num),
                   Ratio1::den / std::gcd(Ratio1::den, Ratio2::den) *
                       Ratio2::den>>
        type;
};
} // namespace std

#endif // CORE_SIZE_HPP
