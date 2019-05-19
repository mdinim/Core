//
// Created by Dániel Molnár on 2019-03-17.
//

#pragma once
#ifndef CORE_VALUEWRAPPER_HPP
#define CORE_VALUEWRAPPER_HPP

#include <variant>

#include <Utils/Utils.hpp>

namespace Core {

/// \brief Wraps the std::variant to enable assignment, equality and type check.
template<class ...Types>
class ValueWrapper : public std::variant<Types...> {
private:
    using Base = std::variant<Types...>;
public:
    /// \brief Default constructor.
    ValueWrapper()  = default;

    /// \brief Construct from value of type T. If the variant can not hold the type, assert is raised.
    template<class T>
    ValueWrapper(const T& value) : Base(value) {
        static_assert(contains<T, Types...>(), "ValueWrapper can not hold the assigned type");
    }

    /// \brief Assign a value. If the variant can not hold the type, assert is raised.
    template<class T>
    ValueWrapper& operator=(const T& value) {
        static_assert(contains<T, Types...>(), "ValueWrapper can not hold the assigned type");

        Base::operator=(Base(value));
        return *this;
    }

    /// \brief Check equality. If the variant does not hold the type, returns obviously false.
    template<class T, typename = std::enable_if_t<contains<T, Types...>()>>
    bool operator==(const T& value) const {
        return std::holds_alternative<T>(*this) && std::get<T>(*this) == value;
    }

    std::variant<Types...>& to_std_variant() {
        return *this;
    }

    const std::variant<Types...>& to_std_variant() const {
        return *this;
    }

    /// \brief For convenience a template type-checker.
    template<class T>
    bool is() const {
        return std::holds_alternative<T>(*this);
    }
};

/// \brief The other side of the equality check to enable expressions e.g.
/// \code
/// bool equals = 2 == ValueWrapper<long>(2l); // false, type mis-match
/// \endcode
template<class ...Types, class T, typename = std::enable_if_t<contains<T, Types...>()>>
bool operator==(const T& value, const ValueWrapper<Types...>& wrapper) {
    return wrapper == value;
}

}

#endif //CORE_VALUEWRAPPER_HPP
