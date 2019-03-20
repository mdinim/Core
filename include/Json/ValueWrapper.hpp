//
// Created by Dániel Molnár on 2019-03-17.
//

#pragma once
#ifndef CORE_VALUEWRAPPER_HPP
#define CORE_VALUEWRAPPER_HPP

#include <variant>

#include <Utils.hpp>

namespace Core {

template<class ...Types>
class ValueWrapper : public std::variant<Types...>{
private:
    using Base = std::variant<Types...>;
public:
    ValueWrapper()  = default;

    template<class T>
    ValueWrapper(const T& value) : Base(value) {
        static_assert(contains<T, Types...>(), "ValueWrapper can not hold the assigned type");
    }

    template<class T>
    ValueWrapper& operator=(const T& value) {
        static_assert(contains<T, Types...>(), "ValueWrapper can not hold the assigned type");

        Base::operator=(Base(value));
        return *this;
    }

    template<class T, typename = std::enable_if_t<contains<T, Types...>()>>
    bool operator==(const T& value) const {
        return std::holds_alternative<T>(*this) && std::get<T>(*this) == value;
    }

    template<class T>
    bool is() const {
        return std::holds_alternative<T>(*this);
    }
};

template<class ...Types, class T, typename = std::enable_if_t<contains<T, Types...>()>>
bool operator==(const T& value, const ValueWrapper<Types...>& wrapper) {
    return wrapper == value;
}

}

#endif //CORE_VALUEWRAPPER_HPP
