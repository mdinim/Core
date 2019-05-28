//
// Created by Dániel Molnár on 2019-03-07.
//

#include "Json.hpp"
#include <cctype>
#include <stack>
#include <string_view>
#include <algorithm>

#include <iostream>
#include <list>
#include <cctype>

#include <Utils/Utils.hpp>

namespace Core {

std::optional<std::string> Json::parse_string(const std::string_view key_string) {
    auto key_end = key_string.begin();
    char current = '\0';
    std::string result;
    do {
        current = *key_end;
        if(current == '\\' && key_end != (key_string.end() - 1)) {
            auto next = *(key_end + 1);
            
            auto allowed = std::vector{'"', '\\', 'b', 'f', 'n', 'r', 't'};
            if (auto found = std::find(allowed.begin(), allowed.end(), next); found != allowed.end()) {
                result += current;
                result += next;
                key_end++;
            } else if (next == 'u' && std::distance(key_end, key_string.end()) > 3) {
                result += current + next;
                int counter = 2;
                
                while (std::isxdigit(*(key_end + counter)) != 0 && counter <= 5) {
                    result += *(key_end + counter);
                    counter++;
                }
                
                if (counter != 6)
                    return std::nullopt;
                
                key_end += 6;
            } else {
                return std::nullopt;
            }
        } else if (current != '"') {
            result += current;
        }
        
        key_end++;
    } while (current != '"');
    
    return result;
}
    
std::optional<Json::ParsedValue> Json::parse_array(const std::string_view array_string) {
    enum class ParseState {
        None,
        Value,
        MaybeComa
    };
    
    auto current_state = ParseState::None;
    auto array_start = array_string.begin();
    
    while (std::isspace(*array_start))
        array_start++;
    
    if(*array_start != '[')
        return std::nullopt;
    
    array_start++;
    auto array = Json::create_array();
    unsigned int processed_characters = 0;
    bool end = false;
    bool more_data = false;
    
    for(auto it = array_start; it != array_string.end() && !end; ++it) {
        const auto& current_character = *it;
        std::string_view inner_view(&(*it), std::distance(it, array_string.end()));
        switch(current_state) {
            case ParseState::None: {
                if (std::isspace(current_character))
                    continue;
                if (current_character == ']') {
                    if (more_data)
                        return std::nullopt;
                    
                    processed_characters = std::distance(array_string.begin(), it);
                    end = true;
                } else
                    current_state = ParseState::Value;
                it--;
                break;
            }
            case ParseState::Value: {
                switch (current_character) {
                    case '[': {
                        auto child_array = Json::parse_array(inner_view);
                        if  (child_array) {
                            array.push_back(child_array->first);
                            it += child_array->second;
                        } else {
                            return std::nullopt;
                        }
                        break;
                    }
                    case '{': {
                        auto object = Json::parse_object(inner_view);
                        if  (object) {
                            array.push_back(object->first);
                            it += object->second;
                        } else {
                            return std::nullopt;
                        }
                        break;
                    }
                    default: {
                        auto value = Json::parse_value(inner_view);
                        if (!value)
                            return std::nullopt;
                        
                        it += value->second - 1;
                        array.push_back(value->first);
                        break;
                    }
                }
                
                more_data = false;
                current_state = ParseState::MaybeComa;
                break;
            }
            case ParseState::MaybeComa: {
                if (current_character == ',') {
                    current_state = ParseState::None;
                    more_data = true;
                } else if (current_character == ']') {
                    end = true;
                    processed_characters = std::distance(array_string.begin(), it);
                    break;
                }
                else if (!std::isspace(current_character))
                    return std::nullopt;
            }
        }
    }
    
    return {ParsedValue{array, processed_characters}};
}
    
std::optional<Json::ParsedValue> Json::parse_object(const std::string_view object_string) {
    enum class ParseState {
        None,
        Key,
        Value,
        DoubleColon,
        MaybeComa
    };
    
    auto current_state = ParseState::None;
    auto object_start = object_string.begin();
    while (std::isspace(*object_start))
        object_start++;
    
    if (*object_start != '{')
        return std::nullopt;
    
    object_start++;
    auto object = Json::create_object();
    
    bool more_data = false;
    std::optional<std::string> key;
    bool end = false;
    unsigned int processed_characters = 0;
    for (auto it = object_start; it != object_string.end() && !end; ++it) {
        const auto& current_character = *it;
        std::string_view inner_view(&(*it), std::distance(it, object_string.end()));
        switch (current_state) {
            case ParseState::None: {
                if (std::isspace(current_character))
                    continue;
                if (current_character == '"')
                    current_state = ParseState::Key;
                
                if (current_character == '}') {
                    if (more_data)
                        return std::nullopt;
                    
                    processed_characters = std::distance(object_string.begin(), it);
                    end = true;
                }
                break;
            }
            case ParseState::Key: {
                key = Json::parse_string(inner_view);
                if(!key)
                    return std::nullopt;
                
                it += key->size();
                current_state = ParseState::DoubleColon;
                break;
            }
            case ParseState::DoubleColon: {
                if (std::isspace(current_character)) {
                    continue;
                }
                if (current_character != ':')
                    return std::nullopt;
                
                current_state = ParseState::Value;
                break;
            }
            case ParseState::Value: {
                if (std::isspace(current_character))
                    continue;
                switch (current_character) {
                    case '{': {
                        auto child_object = Json::parse_object(inner_view);
                        if (!child_object || !key)
                            return std::nullopt;
                        
                        it += child_object->second;
                        
                        object.set(*key, child_object->first);
                        key.reset();
                        
                        break;
                    }
                    case '[': {
                        auto array = Json::parse_array(inner_view);
                        if (!array || !key)
                            return std::nullopt;
                            
                        it += array->second;
                        
                        object.set(*key, array->first);
                        key.reset();
                        
                        break;
                    }
                    default: {
                        auto value = Json::parse_value(inner_view);
                        if (!value || !key)
                            return std::nullopt;
                        object.set(*key, value->first);
                        
                        it += value->second - 1;
                        
                        break;
                    }
                }
                
                more_data = false;
                current_state = ParseState::MaybeComa;
                break;
            }
            case ParseState::MaybeComa: {
                if (current_character == ',') {
                    current_state = ParseState::None;
                    more_data = true;
                } else if (current_character == '}') {
                    end = true;
                    processed_characters = std::distance(object_string.begin(), it);
                    break;
                }
                else if (!std::isspace(current_character))
                    return std::nullopt;
            }
        }
    }

    return {ParsedValue{object, processed_characters}};
}
    
Json::PropList Json::parse_path(std::string path) {
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

std::optional<Json::ParsedValue> Json::parse_value(const std::string_view &value_string) {
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

    std::stack<ParseState> stateStack;
    stateStack.push(ParseState::None);

    bool isWholeNumber = false;
    bool isFractionalNumber = false;
    bool exponentSignProcessed = false;

    bool shouldEnd = false;
    auto valueStart = value_string.begin();
    auto valueEnd = value_string.begin();
    unsigned int processed_characters = 0;
    for(auto it = value_string.begin(); it != value_string.end() && !shouldEnd; ++it)
    {
        const auto &currentState = stateStack.top();
        const auto &character = *it;
        processed_characters = std::distance(value_string.begin(), it);

        switch(currentState) {
            case ParseState::None:
                if(std::isspace(character))
                    continue;

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
                    return {ParsedValue{Null(), processed_characters + 3}};
                }
                return {};
            }
            case ParseState::Bool: {
                using namespace std::string_literals;
                if(std::string_view(&*valueStart, "true"s.size()) == "true") {
                    return {ParsedValue{true, processed_characters + 3}};
                }
                if(std::string_view(&*valueStart, "false"s.size()) == "false") {
                    return {ParsedValue{false, processed_characters + 4}};
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
                        shouldEnd = true;
                        break;
                }
                break;
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
                if(std::isspace(character)) {
                    shouldEnd = true;
                    continue;
                }
                if(std::isdigit(character))
                    continue;
                return {};
            }
            case ParseState::String: {
                auto string = Json::parse_string(
                                 std::string_view(&(*valueStart), std::distance(valueStart, value_string.end()))
                             );
                if (string.has_value())
                    // + 1 is the closing quote
                    return {ParsedValue{string.value(), processed_characters + string.value().size() + 1}};
                else
                    return std::nullopt;
            }
        }
    }

    std::string_view extractedValue(&*valueStart, std::distance(valueStart, valueEnd));

    if(isWholeNumber)
        try {
            return {ParsedValue{std::stoi(std::string(extractedValue)), processed_characters}};
        } catch(std::out_of_range& ex) {
            return {ParsedValue{std::stol(std::string(extractedValue)), processed_characters}};
        }
    if(isFractionalNumber)
        return {ParsedValue{std::stod(std::string(extractedValue)), processed_characters}};
    return std::nullopt;
}

std::optional<Json::Value> Json::_get(Json::PropList prop_list) const {
    if(prop_list.empty())
    {
        return {};
    }

    auto currentProp = std::move(prop_list.front());
    prop_list.erase(prop_list.begin());

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

    if(!prop_list.empty() && value) {
        if(value->is<Json>()) {
            return std::get<Json>(*value)._get(std::move(prop_list));
        }
    }

    return value;
}

Json::Json(const std::string& json_string) : _valid(true) {
    if (auto object = parse_object(json_string); object) {
        _data = std::move(std::get<Json>(object->first)._data);
    } else if (auto array = parse_array(json_string); array) {
        _data = std::move(std::get<Json>(array->first)._data);
    } else {
        _valid = false;
    }
}

std::optional<Json::Value> Json::get(const std::string& path) const {
    return _get(parse_path(path));
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

void Json::print(std::ostream& os, unsigned& tab_count) const {
    const auto printTabs = [&tab_count, &os]() {
        for(unsigned i = 0; i < tab_count; i++)
            os << '\t';
    };

    static const auto valueVisitor = make_overload {
        [&os, &tab_count] (const Json& json) {
            json.print(os, tab_count);
        }, [&os](const std::string& value) {
            os << '"' << value << '"';
        }, [&os](const Null&) {
           os << "null";
        }, [&os](const bool& value) {
            os << std::boolalpha << value << std::noboolalpha;
        }, [&os](const auto& value) {
            os << value;
        }
    };

    auto i = 0u;
    visit_variant(_data, [&os, &i, &printTabs, &tab_count](const Json::JsonArray& array) {
            os << "[" << std::endl;
            tab_count++;
            for(const auto& value : array) {
                printTabs();
                visit_variant(value.to_std_variant(), valueVisitor);
                if(++i != array.size())
                    os << ",";
                os << std::endl;
            }
            tab_count--;
            printTabs();
            os << "]";
        }, [&os, &i, &printTabs, &tab_count](const Json::JsonObject& object) {
            os << "{" << std::endl;
            tab_count++;
            for(const auto& [key, value] : object) {
                printTabs();
                os << "\"" << key << "\"" << ": ";
                visit_variant(value.to_std_variant(), valueVisitor);
                if(++i != object.size())
                    os << ",";
                os << std::endl;
            }
            tab_count--;
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
