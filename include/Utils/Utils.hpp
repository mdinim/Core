//
// Created by Dániel Molnár on 2019-03-12.
//

#ifndef CORE_UTILS_HPP
#define CORE_UTILS_HPP

#include <variant>

namespace Core {
/// \brief Generates a struct that has all the operator()s as the template types
/// do.
template <typename... Ts> struct make_overload : Ts... {
    using Ts::operator()...;
};

/// \brief Deduction guideline
template <typename... Ts> make_overload(Ts...)->make_overload<Ts...>;

template <typename Variant, typename... Alternatives>
decltype(auto) visit_variant(Variant &&variant,
                             Alternatives &&... alternatives) {
    return std::visit(
        make_overload{std::forward<Alternatives>(alternatives)...},
        std::forward<Variant>(variant));
}

/// \brief If the parameter pack Ts contains the T type then the holds the value
/// true
template <typename T, typename... Ts> constexpr bool contains() {
    return std::disjunction_v<std::is_same<T, Ts>...>;
}

/// \brief If the parameter pack Ts contains a type that is convertible from T
/// then holds the value true.
template <typename T, typename... Ts> constexpr bool contains_convertible() {
    return std::disjunction_v<std::is_convertible<T, Ts>...>;
}

/// \brief If the parameter pack Ts contains a type that is assignable with T
/// then holds the value true.
template <typename T, typename... Ts> constexpr bool contains_assignable() {
    return std::disjunction_v<std::is_assignable<Ts, T>...>;
}

/// \brief If the parameter pack Ts contains a type that is constructible with T
/// then holds the value true.
template <typename T, typename... Ts> constexpr bool contains_constructible() {
    return std::disjunction_v<std::is_constructible<Ts, T>...>;
}

template <typename T, typename = std::void_t<>>
struct is_container : public std::false_type {};

template <typename T>
struct is_container<
    T,
    std::void_t<
        typename T::value_type, typename T::size_type, typename T::iterator,
        typename T::const_iterator, decltype(std::declval<T>().size()),
        decltype(std::declval<T>().begin()), decltype(std::declval<T>().end()),
        decltype(std::declval<T>().cbegin()),
        decltype(std::declval<T>().cend())>> : public std::true_type {};
} // namespace Core

#endif // CORE_UTILS_HPP
