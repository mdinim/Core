//
// Created by Dániel Molnár on 2019-03-07.
//

#include "Json/Json.hpp"
#include <cctype>
#include <stack>
#include <string_view>
#include <algorithm>

#include <iostream>
#include <list>

#include <Utils.hpp>

namespace Core {

Json::PropList Json::parsePath(std::string path) {
    PropList propList;

    enum ParseState {
        Prop,
        Index
    };

    auto currentState = ParseState::Prop;
    std::optional<std::string::iterator> propBegin;
    std::optional<std::string::iterator> indexBegin;
    bool escaped = false;
    int indexesPushed = 0;
    auto removeLastNProps = [](int& remainingProps, PropList& propList) {
        while(remainingProps-- > 0)
            propList.erase(--propList.end());
        remainingProps = 0;
    };
    for(auto it = path.begin(); it != path.end(); it++) {
        auto& character = *it;
        if(character == '\\') {
            escaped = true;
            path.erase(it);
        } else {
            escaped = false;
        }

        switch(currentState) {
            case ParseState::Prop: {
                if(!escaped && character == '[') {
                    if(propBegin)
                        propList.emplace_back(std::string(&*propBegin.value(), std::distance(*propBegin, it)));
                    indexBegin = it + 1;
                    currentState = ParseState::Index;
                    indexesPushed = propBegin ? 1 : 0;
                    break;
                }
                if(auto onEnd = it == path.end() - 1; (!escaped && character == '.') || onEnd) {
                    if(propBegin)
                        propList.emplace_back(std::string(&*propBegin.value(),
                                std::distance(*propBegin, onEnd ? it + 1 : it)));
                    propBegin = it + 1;
                    break;
                }
                if(!propBegin) {
                    propBegin = it;
                }
                break;
            }
            case ParseState::Index: {
                if(!escaped && character == '[') {
                    indexBegin = it + 1;
                } else if(!escaped && character == ']') {
                    std::string indexStr(&*indexBegin.value(), std::distance(*indexBegin, it));
                    if(std::all_of(indexStr.begin(), indexStr.end(), [](const auto& c){ return std::isdigit(c);})) {
                        try {
                            auto index = std::stoi(indexStr);

                            propList.emplace_back(index);
                            indexesPushed++;
                        } catch (std::invalid_argument&) {
                            removeLastNProps(indexesPushed, propList);

                            propList.emplace_back(
                                    std::string(&*propBegin.value(), std::distance(*propBegin, it + 1)));
                        }
                    } else {
                        removeLastNProps(indexesPushed, propList);
                        indexesPushed = 0;

                        propList.emplace_back(
                                std::string(&*propBegin.value(), std::distance(*propBegin, it + 1)));
                    }
                } else if(!escaped && character == '.') {
                    currentState = ParseState::Prop;
                    propBegin = it + 1;
                } else if(!std::isdigit(character)) {
                    removeLastNProps(indexesPushed, propList);

                    currentState = ParseState::Prop;
                }
                break;
            }
        }
    }

    return propList;
}

std::optional<Json::Value> Json::parseValue(const std::string_view& valueString) {
    enum class ParseState {
        None,
        Null,
        Bool,
        String,
        WholePart,
        Exponent,
        ExponentSign,
        Fractional,
        LeadingZero
    };

    static auto isSpace = [](const auto& c) { return std::isspace(c); };

    std::stack<ParseState> stateStack;
    stateStack.push(ParseState::None);
    bool isString = false;
    bool isWholeNumber = false;
    bool isFractionalNumber = false;
    bool exponentSignProcessed = false;

    bool escaped = false;
    bool shouldEnd = false;
    auto valueStart = valueString.begin();
    auto valueEnd = valueString.begin();
    for(auto it = valueString.begin(); it != valueString.end(); ++it)
    {
        const auto &currentState = stateStack.top();
        const auto &character = *it;

        switch(currentState) {
            case ParseState::None:
                if(std::isspace(character))
                    continue;
                if(shouldEnd)
                    return {};

                valueStart = it;
                switch(character) {
                    case 'n':
                        stateStack.push(ParseState::Null);
                        continue;
                    case 'f':
                        [[fallthrough]];
                    case 't':
                        stateStack.push(ParseState::Bool);
                        continue;
                    case '"':
                        valueStart++;
                        stateStack.push(ParseState::String);
                        continue;
                    case '-':
                        continue;
                    case '0':
                        stateStack.push(ParseState::LeadingZero);
                        continue;
                    default:
                        if(std::isdigit(character)) {
                            stateStack.push(ParseState::WholePart);
                            it--;
                            continue;
                        }
                        return {};
                }
            case ParseState::Null: {
                using namespace std::string_literals;
                if(std::string_view(&*valueStart, "null"s.size()) == "null") {
                    std::string_view remaining(&*(valueStart+ 4), std::distance(valueStart+4, valueString.end()));
                    if(std::all_of(remaining.begin(), remaining.end(), isSpace))
                        return {Null()};
                }
                return {};
            }
            case ParseState::Bool: {
                using namespace std::string_literals;
                if(std::string_view(&*valueStart, "true"s.size()) == "true") {
                    std::string_view remaining(&*(valueStart+ 4), std::distance(valueStart+4, valueString.end()));
                    if(std::all_of(remaining.begin(), remaining.end(), isSpace))
                        return {true};

                    return {};
                }
                if(std::string_view(&*valueStart, "false"s.size()) == "false") {
                    std::string_view remaining(&*(valueStart+ 5), std::distance(valueStart+5, valueString.end()));
                    if(std::all_of(remaining.begin(), remaining.end(), isSpace))
                        return {false};
                    return {};
                }
                return {};
            }
            case ParseState::LeadingZero:
                if(std::isspace(character)) {
                    shouldEnd = true;
                    continue;
                }
                valueEnd = it + 1;
                if(character == '.') {
                    stateStack.pop();
                    stateStack.push(ParseState::Fractional);
                    continue;
                } else
                    return {};
            case ParseState::WholePart:
                isWholeNumber = true;
                if(std::isspace(character)) {
                    shouldEnd = true;
                    continue;
                }
                valueEnd = it + 1;
                switch(character) {
                    case '.':
                        stateStack.pop();
                        stateStack.push(ParseState::Fractional);
                        continue;
                    case 'e':
                        [[fallthrough]];
                    case 'E':
                        stateStack.pop();
                        stateStack.push(ParseState::ExponentSign);
                        continue;
                    default:
                        if(std::isdigit(character))
                            continue;
                        return {};
                }
            case ParseState::Fractional: {
                isFractionalNumber = true;
                isWholeNumber = false;
                if(std::isspace(character)) {
                    shouldEnd = true;
                    continue;
                }
                valueEnd = it + 1;
                switch(character) {
                    case 'e':
                        [[fallthrough]];
                    case 'E':
                        stateStack.pop();
                        stateStack.push(ParseState::ExponentSign);
                        continue;
                    default:
                        if(std::isdigit(character))
                            continue;
                        return {};
                }
            }
            case ParseState::ExponentSign:
                if(character == '-' || character == '+') {
                    exponentSignProcessed = true;
                    stateStack.push(ParseState::Exponent);
                    continue;
                }
                if(std::isdigit(character))
                    continue;
                return {};
            case ParseState::Exponent: {
                if(std::isspace(character) && !exponentSignProcessed)
                    return {};
                valueEnd = it + 1;
                if(std::isspace(character))
                    shouldEnd = true;
                if(std::isdigit(character))
                    continue;
                return {};
            }
            case ParseState::String: {
                isString = true;
                if(character == '"' && !escaped) {
                    valueEnd = it;
                    stateStack.pop();
                    shouldEnd = true;
                }
                if(character == '\\') {
                    escaped = true;
                    continue;
                } else {
                    escaped = false;
                }
            }
        }
    }

    std::string_view extractedValue(&*valueStart, std::distance(valueStart, valueEnd));

    if(isWholeNumber)
        try {
            return std::stoi(std::string(extractedValue));
        } catch(std::out_of_range& ex) {
            return std::stol(std::string(extractedValue));
        }
    if(isFractionalNumber)
        return {std::stod(std::string(extractedValue))};
    if(isString)
        return {std::string(extractedValue)};
    return {};
}

std::optional<Json::Value> Json::_get(Json::PropList propList) const {
    if(propList.empty())
    {
        return {};
    }

    auto currentProp = std::move(propList.front());
    propList.erase(propList.begin());

    auto value = visit_variant(currentProp, [this](const std::string& prop) {
        return visit_variant(_data, [&prop](const JsonObject& object) -> MaybeValue {
            if(auto it = object.find(prop); it != object.end()) {
                return it->second;
            }
            return std::nullopt;
        }, [](const JsonArray&) -> MaybeValue {
            return std::nullopt;
        });
    }, [this](const std::size_t& prop) {
        return visit_variant(_data, [](const JsonObject&) -> MaybeValue {
            return std::nullopt;
        }, [&prop](const JsonArray& array) -> MaybeValue {
            if(prop < array.size())
                return array.at(prop);

            return std::nullopt;
        });
    });

    if(!propList.empty() && value) {
        if(value->is<Json>()) {
            return std::get<Json>(*value)._get(std::move(propList));
        }
    }

    return value;
}

Json::Json(const std::string& jsonString) : _valid(true) {
    enum class ParseState {
        None,
        Array,
        Object,
        Key,
        Value,
        String
    };

    static const auto isArray = [](const ValueContainer& container) {
        return std::holds_alternative<JsonArray>(container);
    };

    static const auto isObject = [](const ValueContainer& container) {
        return std::holds_alternative<JsonObject>(container);
    };

    bool escaped = false;

    std::stack<ParseState> stateStack;
    stateStack.push(ParseState::None);
    std::stack<ValueContainer> containerStack;
    std::stack<std::string_view> keyStack;

    auto stringStart = jsonString.begin();
    bool moreDataShouldCome = false;
    std::optional<std::string::const_iterator> valueStart;
    std::optional<std::string::const_iterator> valueEnd;

    for(auto it = jsonString.begin(); it != jsonString.end() && _valid; ++it) {
        const auto &currentState = stateStack.top();
        const auto &character = *it;

        if ((currentState == ParseState::Array || currentState == ParseState::Object) && std::isspace(character)) {
            continue;
        }

        switch(currentState)
        {
            // Initial state, check if object or array
            case ParseState::None: {
                switch(character) {
                    case '{':
                        stateStack.push(ParseState::Object);
                        containerStack.emplace(JsonObject());
                        continue;
                    case '[':
                        stateStack.push(ParseState::Array);
                        containerStack.emplace(JsonArray());
                        continue;
                    default:
                        _valid = false;
                        continue;
                }
            }
            case ParseState::Object: {
                switch(character)
                {
                    // Key starts (and a string)
                    case '"':
                        stateStack.push(ParseState::Key);
                        stateStack.push(ParseState::String);
                        stringStart = it + 1;
                        continue;
                    // Finished object
                    case '}': {
                        auto currentContainer = std::move(containerStack.top());
                        // Object should the last container added
                        if(!isObject(currentContainer)) {
                            _valid = false;
                            continue;
                        }
                        containerStack.pop();
                        if (keyStack.empty()) {
                            _data = std::move(currentContainer);
                        } else {
                            auto currentJson = Json::createObject();
                            currentJson._data = std::move(currentContainer);
                            std::visit(DataVisitor(currentJson, keyStack), containerStack.top());
                        }

                        // The last value indicated that there is more to come (, afterwards)
                        if (moreDataShouldCome) {
                            _valid = false;
                            continue;
                        }
                        stateStack.pop();
                        continue;
                    }
                    default:
                        return;
                }
            }
            case ParseState::Array: {
                switch(character)
                {
                    // Object starts
                    case '{':
                        containerStack.emplace(JsonObject());
                        stateStack.push(ParseState::Object);
                        continue;
                    // Array of arrays
                    case '[':
                        containerStack.emplace(JsonArray());
                        stateStack.push(ParseState::Array);
                        continue;
                    // String value starts, but let the value state handle it
                    case '"':
                        stateStack.push(ParseState::Value);
                        it--;
                        continue;
                    // Array closed
                    case ']': {
                        auto currentContainer = std::move(containerStack.top());
                        containerStack.pop();
                        // Array should be on top
                        if(!isArray(currentContainer)) {
                            _valid = false;
                            continue;
                        }
                        if (keyStack.empty()) {
                            _data = std::move(currentContainer);
                        } else {
                            auto currentJson = Json::createObject();
                            currentJson._data = std::move(currentContainer);
                            std::visit(DataVisitor(currentJson, keyStack), containerStack.top());
                        }

                        // The last value indicated that there is more to come (, afterwards)
                        if (moreDataShouldCome) {
                            _valid = false;
                            continue;
                        }
                        stateStack.pop();
                        continue;
                    }
                    default:
                        stateStack.push(ParseState::Value);
                        it--;
                        continue;
                }
            }
            case ParseState::Key: {
                moreDataShouldCome = false;
                switch(character) {
                    // Key "closer", Value comes
                    case ':': {
                        std::string_view key(&*stringStart, std::distance(stringStart, it) - 1);

                        keyStack.push(key);
                        stateStack.pop();
                        stateStack.push(ParseState::Value);
                        valueStart.reset();
                        continue;
                    }
                    default:
                        break;
                }
                break;
            }
            case ParseState::Value: {
                moreDataShouldCome = false;
                switch(character) {
                    case '"':
                        valueStart = it;
                        stringStart = it + 1;
                        stateStack.push(ParseState::String);
                        continue;
                    case '}':
                        // Object closed, object should be on top
                        if(!isObject(containerStack.top())) {
                            _valid = false;
                            continue;
                        }

                        [[fallthrough]];
                    case ']':
                        // Array closed, array should be on top
                        if(character == ']' && !isArray(containerStack.top())) {
                            _valid = false;
                            continue;
                        }

                        // The value ends here
                        if (!valueEnd)
                            valueEnd = it;
                        [[fallthrough]];
                    case ',': {
                        if (valueStart && !valueEnd)
                            valueEnd = it;
                        if (valueStart && valueEnd) {
                            std::string_view value(&*(*valueStart), std::distance(*valueStart, *valueEnd));

                            auto parsedValue = parseValue(value);
                            if(!parsedValue) {
                                _valid = false;
                                continue;
                            }
                            std::visit(DataVisitor(*parsedValue, keyStack), containerStack.top());
                        }
                        stateStack.pop();
                        valueStart.reset();
                        valueEnd.reset();

                        if (character == ']' || character == '}')
                            // Let the previous state (Object or Array) close itself properly.
                            it--;

                        // Indicate that there should be at least one more key/value
                        moreDataShouldCome = character == ',';

                        continue;
                    }
                    case '{':
                        stateStack.push(ParseState::Object);
                        containerStack.emplace(JsonObject());
                        continue;
                    case '[':
                        stateStack.push(ParseState::Array);
                        containerStack.emplace(JsonArray());
                        continue;
                    default:
                        // Skip whitespaces
                        if (!valueStart && std::isspace(character))
                            continue;

                        // We already found the end of the value, only whitespaces should be afterwards
                        if (valueEnd && !std::isspace(character)) {
                            std::string key = std::string(keyStack.top());
                            std::string value(&*(*valueStart), std::distance(*valueStart, *valueEnd));

                            _valid = false;
                            continue;
                        }

                        // This is a whitespace, close the value here
                        if (valueStart && !valueEnd && std::isspace(character)) {
                            valueEnd = it;
                            continue;
                        }

                        // Not a whitespace character, value starts here
                        if (!valueStart && !std::isspace(character)) {
                            valueStart = it;
                            continue;
                        }
                        break;
                }
                break;
            }
            case ParseState::String: {
                switch(character) {
                    case '"':
                        if(!escaped)
                            stateStack.pop();
                        escaped = false;
                        continue;
                    default:
                        escaped = character == '\\';
                        break;
                }
                break;
            }
        }
    }

    _valid = stateStack.top() == ParseState::None;
    if(!_valid)
        visit_variant(_data, [](auto& data) {
            data.clear();
        });
}

std::optional<Json::Value> Json::get(const std::string& path) const {
    return _get(parsePath(path));
}

std::size_t Json::size() const {
    return visit_variant(_data, [](const auto& container) {
        return container.size();
    });
}

bool Json::operator==(const Core::Json &other) const {
    if(_data.index() != other._data.index())
        return false;

    return visit_variant(_data, [&other](const JsonArray& array) {
        const auto& otherArray = std::get<JsonArray>(other._data);
        return array == otherArray;
    }, [&other](const JsonObject& object) {
        const auto& otherObject = std::get<JsonObject>(other._data);
        return object == otherObject;
    });
}

void Json::print(std::ostream& os, unsigned& tabCount) const {
    const auto printTabs = [&tabCount, &os]() {
        for(unsigned i = 0; i < tabCount; i++)
            os << '\t';
    };

    static const auto valueVisitor = make_overload {
        [&os, &tabCount] (const Json& json) {
            json.print(os, tabCount);
        }, [&os](const std::string& value) {
            os << '"' << value << '"';
        }, [&os](const Null& value) {
           os << "null";
        }, [&os](const auto& value) {
            os << value;
        }
    };

    auto i = 0u;
    visit_variant(_data, [&os, &i, &printTabs, &tabCount](const Json::JsonArray& array) {
            os << "[" << std::endl;
            tabCount++;
            for(const auto& value : array) {
                printTabs();
                visit_variant(value.toStdVariant(), valueVisitor);
                if(++i != array.size())
                    os << ",";
                os << std::endl;
            }
            tabCount--;
            printTabs();
            os << "]";
        }, [&os, &i, &printTabs, &tabCount](const Json::JsonObject& object) {
            os << "{" << std::endl;
            tabCount++;
            for(const auto& [key, value] : object) {
                printTabs();
                os << key << ": ";
                visit_variant(value.toStdVariant(), valueVisitor);
                if(++i != object.size())
                    os << ",";
                os << std::endl;
            }
            tabCount--;
            printTabs();
            os << "}";
        }
    );
}

Json::Value& Json::operator[](const std::string &property) {
    if(!std::holds_alternative<JsonObject>(_data)) {
        throw bad_json_access("Not a JSON Object");
    }

    auto& object = std::get<JsonObject>(_data);

    return object[property];
}

const Json::Value& Json::at(const std::string &property) const {
    if(!std::holds_alternative<JsonObject>(_data)) {
        throw bad_json_access("Not a JSON Array");
    }

    auto& object = std::get<JsonObject>(_data);

    return object.at(property);
}

Json::Value& Json::at(const std::string &property) {
    if(!std::holds_alternative<JsonObject>(_data)) {
        throw bad_json_access("Not a JSON Object");
    }

    auto& object = std::get<JsonObject>(_data);

    return object.at(property);
}


const Json::Value& Json::at(std::size_t index) const {
    if(!std::holds_alternative<JsonArray>(_data)) {
        throw bad_json_access("Not a JSON Array");
    }

    auto& array = std::get<JsonArray>(_data);

    return array.at(index);
}

Json::Value& Json::at(std::size_t index) {
    if(!std::holds_alternative<JsonArray>(_data)) {
        throw bad_json_access("Not a JSON Object");
    }

    auto& array = std::get<JsonArray>(_data);

    return array.at(index);
}

const Json::Value& Json::operator[](std::size_t index) const {
    if(!std::holds_alternative<JsonArray>(_data)) {
        throw bad_json_access("Not a JSON Array");
    }

    auto& array = std::get<JsonArray>(_data);

    return array[index];
}

Json::Value& Json::operator[](std::size_t index) {
    if(!std::holds_alternative<JsonArray>(_data)) {
        throw bad_json_access("Not a JSON Array");
    }

    auto& array = std::get<JsonArray>(_data);

    return array[index];
}

void Json::push_back(const Json::Value& value) {
    if(!std::holds_alternative<JsonArray>(_data)) {
        throw bad_json_access("Not a JSON Array");
    }

    auto& array = std::get<JsonArray>(_data);

    array.push_back(value);
}

void Json::pop_back() {
    if(!std::holds_alternative<JsonArray>(_data)) {
        throw bad_json_access("Not a JSON Array");
    }

    auto& array = std::get<JsonArray>(_data);

    array.pop_back();
}

std::ostream& operator<<(std::ostream& os, const Json& json) {
    unsigned tabCount = 0;
    json.print(os, tabCount);
    return os;
}

}