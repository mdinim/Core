//
// Created by Dániel Molnár on 2019-03-07.
//

#ifndef CORE_JSON_HPP
#define CORE_JSON_HPP

#include <variant>
#include <optional>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <stack>
#include <map>

#include <Utils.hpp>

namespace Core {

class Json {
private:
    using PropList = std::list<std::variant<std::string, std::size_t>>;

    using Value = std::variant<bool, int, double, std::string, Json>;
    using MaybeValue = std::optional<Value>;
    using JsonObject = std::map<std::string, Value>;
    using JsonArray = std::vector<Value>;
    using ValueContainer = std::variant<JsonObject, JsonArray>;

    ValueContainer _data;

    struct DataVisitor {
        explicit DataVisitor(const Value& value, std::stack<std::string_view>& keyStack) : value(value), keyStack(keyStack) {}
        const Value& value;
        std::stack<std::string_view>& keyStack;
        void operator()(JsonObject& container) {
            container.emplace(keyStack.top(), value);
            keyStack.pop();
        }

        void operator()(JsonArray& container) {
            container.push_back(value);
        }
    };

    bool _valid = false;

    static PropList parsePath(std::string path);

    static std::optional<Value> parseValue(const std::string_view& valueString);

    std::optional<Value> _get(PropList propList) const;

    void print(std::ostream& os, unsigned& tabCount) const;

    template<class T>
    void _set(PropList& propList, const T& value) {
        auto currentProperty = std::move(propList.front());
        propList.erase(propList.begin());

        auto data = visit_variant(_data, [&currentProperty] (JsonObject& object) {
            if(!std::holds_alternative<std::string>(currentProperty)) {
                throw 0;
            }
            auto& key = std::get<std::string>(currentProperty);
            if(object.find(key) == object.end()) {
                object.emplace(key, Value{});
            }
            return std::ref(object.at(key));
        }, [&currentProperty] (JsonArray& array) {
            if(!std::holds_alternative<std::size_t>(currentProperty) ||
                std::get<std::size_t>(currentProperty) >= array.size()) {
                throw 0;
            }
            return std::ref(array.at(std::get<std::size_t>(currentProperty)));
        });

        if(propList.empty())
            data.get() = Value(value);
        else
            visit_variant(data.get(), [&propList, &value](Json& json) {
                json._set(propList, value);
            }, [](const auto& prop) {
                throw 0;
            });
    }

public:
    Json() : _data(JsonObject()), _valid(true) {}

    Json(const std::string& jsonString);

    std::size_t size() const;

    template<class T>
    T get(const std::string &path, T fallbackValue) const {
        auto optional = get<T>(path);
        return optional ? *optional : fallbackValue;
    }

    template<class T>
    std::optional<T> get(const std::string &path) const {
        auto variant = get(path);
        if(variant && std::holds_alternative<T>(*variant))
            return std::get<T>(*variant);
        return {};
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
    std::optional<Value> get(const std::string& path) const;

    template<class T>
    void set(const std::string &path, const T& value) {
        PropList propList = parsePath(path);

        _set(propList, value);
    }

    Value operator[](const std::string& property) {
        if(!std::holds_alternative<JsonObject>(_data)) {
            throw 0;
        }

        auto& object = std::get<JsonObject>(_data);

        return object[property];
    }

    explicit operator bool() const {
        return valid();
    }

    bool operator==(const Json& other) const;

    bool operator!=(const Json& other) const {
        return !(*this == other);
    }

    bool valid() const {
        return _valid;
    }

    friend std::ostream& operator<<(std::ostream& os, const Json& json);
};

}

#endif //CORE_JSON_HPP
