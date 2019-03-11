//
// Created by Dániel Molnár on 2019-03-07.
//

#ifndef CORE_JSON_HPP
#define CORE_JSON_HPP

#include <variant>
#include <optional>
#include <vector>
//TODO: REMOVE THIS
#include <iostream>
#include <string>
#include <stack>
#include <map>

namespace Core {

class Json {
private:
    enum class ParseState {
        None,
        Array,
        Object,
        Key,
        Value,
        String
    };
    enum class ValueParseState {
        None,
        Bool,
        String,
        WholePart,
        Exponent,
        ExponentSign,
        Fractional,
        LeadingZero
    };
    using Value = std::variant<bool, int, double, std::string, Json>;
    using ValueContainer = std::variant<std::map<std::string, Value>, std::vector<Value>>;

    ValueContainer _data;

    struct DataVisitor {
        explicit DataVisitor(const Value& value, std::stack<std::string_view>& keyStack) : value(value), keyStack(keyStack) {}
        const Value& value;
        std::stack<std::string_view>& keyStack;
        void operator()(std::map<std::string, Value>& container) {
            container.emplace(keyStack.top(), value);
            keyStack.pop();
        }
        void operator()(std::vector<Value>& container) {
            container.push_back(value);
        }
    };

    bool _valid = false;
    std::optional<Value> parseValue(const std::string_view& valueString) const;
    Json(ValueContainer&& container) : _data(std::move(container)), _valid(true) {}
public:
    Json(const std::string& jsonString);


    template<class T>
    T get(const std::string &path, T fallbackValue) const {

    }

    template<class T>
    std::optional <T> get(const std::string &path) const {

    }

    /// \brief Get part of the json.
    /// \param path Path in the json object.
    /// Given the json string:
    /// {
    ///     "name": "Sandor",
    ///     "age": 20,
    ///     "acquaintances": [
    ///         {
    ///             "name": "Rosie",
    ///             "age": 46,
    ///             "children": [
    ///                 "name": "Billy",
    ///                 "age": 10
    ///             ]
    ///         },
    ///         {
    ///             "name": "Sheryl",
    ///             "age": 20
    ///         }
    ///     ]
    /// }
    ///
    /// The path of "acquaintances[0]" returns the json representation of:
    ///
    /// {
    ///     "name": "Rosie",
    ///     "age": 46,
    ///     "children": [
    ///         "name": "Billy",
    ///         "age": 10
    ///     ]
    /// }
    std::optional<Json> getNode(const std::string &path) const;

    template<class T>
    void set(const std::string &path, const T& value) {

    }

    std::string toString() const;

    bool valid() const {
        return _valid;
    }
};

}

#endif //CORE_JSON_HPP
