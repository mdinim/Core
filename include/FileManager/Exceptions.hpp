//
// Created by Dániel Molnár on 2019-07-27.
//

#ifndef CORE_EXCEPTIONS_HPP
#define CORE_EXCEPTIONS_HPP

namespace Core::Exceptions {
class InvalidPath : public std::runtime_error {
  public:
    explicit InvalidPath(const std::string &what_arg)
        : std::runtime_error(what_arg) {}
};
} // namespace Core::FileManager

#endif // CORE_EXCEPTIONS_HPP
