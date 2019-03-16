//
// Created by Dániel Molnár on 2019-03-12.
//

#ifndef CORE_UTILS_HPP
#define CORE_UTILS_HPP

#include <variant>

namespace Core {
    /// \brief Generates a struct that has all the operator()s as the template types do.
    template<typename... Ts> struct make_overload: Ts... { using Ts::operator()...; };

    /// \brief Deduction guideline
    template<typename... Ts> make_overload(Ts...) -> make_overload<Ts...>;

    template<typename Variant, typename... Alternatives>
    decltype(auto) visit_variant(Variant&& variant, Alternatives&&... alternatives) {
        return std::visit(
                make_overload{std::forward<Alternatives>(alternatives)...},
                std::forward<Variant>(variant)
            );
    }
}

#endif //CORE_UTILS_HPP
