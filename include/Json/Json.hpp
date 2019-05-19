//
// Created by Dániel Molnár on 2019-03-07.
//

#ifndef CORE_JSON_HPP
#define CORE_JSON_HPP

#include <variant>
#include <optional>
#include <functional>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <stack>
#include <map>

#include "Utils/ValueWrapper.hpp"

namespace Core {

/// \brief Exception that is thrown in case bad access occurs in when dealing with JSONs.
class bad_json_access : public std::exception {
private:
    std::string _message;
public:
    bad_json_access(std::string message) : _message(std::move(message)) {}
    const char * what() const noexcept {
        return _message.c_str();
    }
};

/// \brief JSON object representation.
/// Provides means to handle JSON objects, parses, prettyfies, prints in the appropriate manner.
class Json {
public:
    /// \brief Representation of Null values.
    struct Null {
        bool operator==(const Null&) const {
            return true;
        }
    };
private:
    using PropList = std::list<std::variant<std::string, std::size_t>>;

    using Value = ValueWrapper<Null, bool, int, long, double, std::string, Json>;
    using MaybeValue = std::optional<Value>;
    using JsonObject = std::map<std::string, Value>;
    using JsonArray = std::vector<Value>;
    using ValueContainer = std::variant<JsonObject, JsonArray>;

    /// \brief Internal representatiton of the contained data.
    ValueContainer _data;

    /// \brief Visitor for the possible containers.
    /// Upon visitation, it emplaces the value into the received container.
    struct DataVisitor {
        /// \brief Construct the visitor
        explicit DataVisitor(const Value& value, std::stack<std::string_view>& keyStack) : value(value), keyStack(keyStack) {}

        /// \brief The value to add to the container.
        const Value& value;
        /// \brief The stack that holds the keys. The top-most value will be used as key if visited by a JSON Object.
        std::stack<std::string_view>& keyStack;

        /// \brief Adds the value with the top-most key in the keystack to the internal map.
        void operator()(JsonObject& container) {
            container.emplace(keyStack.top(), value);
            keyStack.pop();
        }

        /// \brief Pushes the value to the array.
        void operator()(JsonArray& container) {
            container.push_back(value);
        }
    };

    /// \brief Flag to represent if the construction was successful.
    bool _valid = false;

    /// \brief Parses the path received in path.
    /// Produces a mix of std::size_t and std::string values.
    /// Internal use only.
    /// \see Json::get
    static PropList parse_path(std::string path);

    /// \brief Parses the value received in value_string.
    /// \return optionally the Value received in the string.
    /// Parses strings like "Hello World", null, true, false, 1e-35 that conform to the JSON specification.
    static std::optional<Value> parse_value(const std::string_view &value_string);

    /// \brief Get the property represented by the property list propList
    /// Internal use only to avoid copying the list multiple times.
    /// \see Json::get
    std::optional<Value> _get(PropList prop_list) const;

    /// \brief Pretty-print the JSON
    /// \see Json::operator<<
    void print(std::ostream& os, unsigned& tab_count) const;

    /// \brief Set the value in the object with the path propList.
    /// Internal use only to avoid the mis-usage of propList.
    /// \see Json::set
    template<class T>
    void _set(PropList& prop_list, const T& value) {
        const auto current_property = std::move(prop_list.front());
        prop_list.erase(prop_list.begin());

        auto data = visit_variant(_data,
            // Get the data by the key
            [&current_property] (JsonObject& object) {
                if(!std::holds_alternative<std::string>(current_property))
                    throw bad_json_access("Can not set index in an Object");

                const auto& key = std::get<std::string>(current_property);
                if(object.find(key) == object.end())
                    object.emplace(key, Value{});

                return std::ref(object.at(key));
            },
            // Get the data by the index
            [&current_property] (JsonArray& array) {
                if(!std::holds_alternative<std::size_t>(current_property))
                    throw bad_json_access("Can not set key in an Array");

                const auto& index = std::get<std::size_t>(current_property);
                if(auto desiredSize = index + 1; desiredSize > array.size())
                    array.resize(desiredSize, Null());

                return std::ref(array.at(index));
            }
        );

        // If no further property, set value
        if(prop_list.empty())
            data.get() = Value(value);
        else {
            // If the data is not an object or array, change it to be
            if(!data.get().template is<Json>())
                data.get() = visit_variant(prop_list.front(),
                    // if the next property is a key, create object
                    [](const std::string& ) {
                        return Json::create_object();
                    },
                    // if the next property is an index, create array
                    [](const int&) {
                        return Json::create_array();
                    }
                );

            // Visit the next one
            visit_variant(data.get().to_std_variant(), [&prop_list, &value](Json &json) {
                json._set(prop_list, value);
            }, [](const auto&) {
                // should never happen
            });
        }
    }

    /// \brief Create a JSON array with data provided in array.
    /// \see Json::createArray
    Json(const JsonArray& array) : _data(array), _valid(true) {}

    /// \brief Create a JSON object with the data provided in object.
    /// \see Json::createObject
    Json(const JsonObject& object) : _data(object), _valid(true) {}
public:
    /// \brief Parse json_string.
    /// Afterwards the validity flag is set accordingly.
    /// \see Json::isValid
    Json(const std::string& json_string);

    /// \brief Check the size of the underlying object.
    /// In case of Objects, it returns the number of keys, in case of array, it returns the number of data held.
    std::size_t size() const;

    /// \brief Retrieve the value currently held in the object/array.
    /// Accepted path structure: <objectKey>[<arrayIndex>].<objectKey2>[<arrayIndex2>]...
    /// \param fallback_value This value gets returned if the key sequence is not available in the object.
    template<class T>
    T get(const std::string &path, T fallback_value) const {
        auto optional = get<T>(path);
        return optional ? *optional : fallback_value;
    }

    /// \brief Optionally retrieve the value currently held in the object/array.
    /// \see Json::get
    template<class T>
    std::optional<T> get(const std::string &path) const {
        auto variant = get(path);
        if(variant && std::holds_alternative<T>(*variant))
            return std::get<T>(*variant);
        return {};
    }

    /// \brief Get part of the json.
    /// \param path Path in the json object.
    /// \code
    /// // Given the json string:
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
    /// // The path of "acquaintances[0]" returns the json representation of:
    ///
    /// {
    ///     "name": "Rosie",
    ///     "age": 46,
    ///     "children": [
    ///         "name": "Billy",
    ///         "age": 10
    ///     ]
    /// }
    /// \endcode
    std::optional<Value> get(const std::string& path) const;

    /// \brief Set value based on path.
    /// If a key does not exist in an object, it gets created with the appropriate type.
    /// Arrays are filled with null until the desired index, existing keys/indexes are overwritten.
    ///
    /// E.g.:
    /// \code
    /// auto object = Json::create_object();
    /// object.set("hello.world[2]", 2);
    /// std::cout << object << std::endl;
    /// // {
    /// //     "hello" : {
    /// //         "world": [
    /// //             null,
    /// //             2
    /// //         ]
    /// //     }
    /// // }
    /// \endcode
    template<class T>
    void set(const std::string &path, const T& value) {
        PropList prop_list = parse_path(path);

        _set(prop_list, value);
    }

    /// \brief Object accessor
    /// \throws bad_json_access if not an object
    const Value& at(const std::string& property) const;

    /// \brief Object accessor
    /// \throws bad_json_access if not an object
    Value& at(const std::string& property);

    /// \brief Object accessor. If the property does not exists, it gets created with null value.
    /// \throws bad_json_access if not an object
    Value& operator[](const std::string& property);

    /// \brief Array accessor.
    /// \throws bad_json_access if not an array
    /// \throws std::out_of_range if index >= size()
    const Value& at(std::size_t index) const;

    /// \brief Array accessor.
    /// \throws bad_json_access if not an array
    /// \throws std::out_of_range if index >= size()
    Value& at(std::size_t index);

    /// \brief Array accessor.
    /// \throws bad_json_access if not an array
    Value& operator[](std::size_t index);

    /// \brief Array accessor.
    /// \throws bad_json_access if not an array
    const Value& operator[](std::size_t index) const;

    /// \brief Push an object to the end of the array.
    /// \throws bad_json_access if not an array
    void push_back(const Value& value);

    /// \brief Pop the object in the end of the array.
    /// \throws bad_json_access if not an array
    void pop_back();

    /// \brief Validity check for conveinence.
    /// \see Json::valid
    explicit operator bool() const {
        return valid();
    }

    /// \brief Equality check for Json objects.
    bool operator==(const Json& other) const;

    /// \brief Inequality check for Json objects.
    bool operator!=(const Json& other) const {
        return !(*this == other);
    }

    /// \brief Validity check. \returns true if the string holds meaningful data.
    bool valid() const {
        return _valid;
    }

    /// \brief Pretty-print the object.
    friend std::ostream& operator<<(std::ostream& os, const Json& json);

    /// \brief Factory function, creates empty object.
    /// Equivalent to:
    /// \code
    /// Json json("{}");
    /// \endcode
    static Json create_object() {
        return Json(JsonObject());
    }

    /// \brief Factory function, creates empty array.
    /// Equivalent to:
    /// \code
    /// Json json("[]");
    /// \endcode
    static Json create_array() {
        return Json(JsonArray());
    }
};

}

#endif //CORE_JSON_HPP
