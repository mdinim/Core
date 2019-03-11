//
// Created by Dániel Molnár on 2019-03-07.
//

#include "Json.hpp"

#include <cctype>
#include <stack>
#include <string_view>
#include <algorithm>

#include <iostream>

namespace Core {

std::optional<Json::Value> Json::parseValue(const std::string_view& valueString) const {
    std::stack<ValueParseState> stateStack;
    stateStack.push(ValueParseState::None);
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
            case ValueParseState::None:
                if(std::isspace(character))
                    continue;
                if(shouldEnd)
                    return {};

                valueStart = it;
                switch(character) {
                    case 'f':
                        [[fallthrough]];
                    case 't':
                        stateStack.push(ValueParseState::Bool);
                        continue;
                    case '"':
                        valueStart++;
                        stateStack.push(ValueParseState::String);
                        continue;
                    case '-':
                        continue;
                    case '0':
                        stateStack.push(ValueParseState::LeadingZero);
                        continue;
                    default:
                        if(std::isdigit(character)) {
                            stateStack.push(ValueParseState::WholePart);
                            it--;
                            continue;
                        }
                        return {};
                }
            case ValueParseState::Bool: {
                using namespace std::string_literals;
                auto isSpace = [](const auto& c) { return std::isspace(c); };
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
            case ValueParseState::LeadingZero:
                if(std::isspace(character)) {
                    shouldEnd = true;
                    continue;
                }
                valueEnd = it + 1;
                if(character == '.') {
                    stateStack.pop();
                    stateStack.push(ValueParseState::Fractional);
                    continue;
                } else
                    return {};
            case ValueParseState::WholePart:
                isWholeNumber = true;
                if(std::isspace(character)) {
                    shouldEnd = true;
                    continue;
                }
                valueEnd = it + 1;
                switch(character) {
                    case '.':
                        stateStack.pop();
                        stateStack.push(ValueParseState::Fractional);
                        continue;
                    case 'e':
                        [[fallthrough]];
                    case 'E':
                        stateStack.pop();
                        stateStack.push(ValueParseState::ExponentSign);
                        continue;
                    default:
                        if(std::isdigit(character))
                            continue;
                        return {};
                }
            case ValueParseState::Fractional: {
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
                        stateStack.push(ValueParseState::ExponentSign);
                        continue;
                    default:
                        if(std::isdigit(character))
                            continue;
                        return {};
                }
            }
            case ValueParseState::ExponentSign:
                if(character == '-' || character == '+') {
                    exponentSignProcessed = true;
                    stateStack.push(ValueParseState::Exponent);
                    continue;
                }
                if(std::isdigit(character))
                    continue;
                return {};
            case ValueParseState::Exponent: {
                if(std::isspace(character) && !exponentSignProcessed)
                    return {};
                if(std::isspace(character))
                    shouldEnd = true;
                if(std::isdigit(character))
                    continue;
                return {};
            }
            case ValueParseState::String: {
                isString = true;
                if(character == '\\') {
                    escaped = true;
                    continue;
                }
                if(character == '"' && !escaped) {
                    valueEnd = it;
                    stateStack.pop();
                    shouldEnd = true;
                }
            }
        }
    }

    std::string_view extractedValue(&*valueStart, std::distance(valueStart, valueEnd));

    if(isWholeNumber)
        return {std::stoi(std::string(extractedValue))};
    if(isFractionalNumber)
        return {std::stod(std::string(extractedValue))};
    if(isString)
        return {std::string(extractedValue)};
    return {};
}

Json::Json(const std::string& jsonString) {
    bool escaped = false;

    std::stack<ParseState> stateStack;
    stateStack.push(ParseState::None);
    std::stack<ValueContainer> containerStack;
    std::stack<std::string_view> keyStack;

    auto stringStart = jsonString.begin();
    bool moreDataShouldCome = false;
    std::optional<std::string::const_iterator> valueStart;
    std::optional<std::string::const_iterator> valueEnd;

    for(auto it = jsonString.begin(); it != jsonString.end(); ++it) {
        const auto &currentState = stateStack.top();
        const auto &character = *it;

        if ((currentState == ParseState::Array || currentState == ParseState::Object) && std::isspace(character)) {
            continue;
        }

        switch(currentState)
        {
            case ParseState::None: {
                switch(character) {
                    case '{':
                        stateStack.push(ParseState::Object);
                        containerStack.emplace(std::map<std::string, Value>());
                        continue;
                    case '[':
                        stateStack.push(ParseState::Array);
                        containerStack.emplace(std::vector<Value>());
                        continue;
                    default:
                        return;
                }
            }
            case ParseState::Object: {
                switch(character)
                {
                    case '"':
                        stateStack.push(ParseState::Key);
                        stateStack.push(ParseState::String);
                        stringStart = it + 1;
                        continue;
                    case '}': {
                        auto currentContainer = std::move(containerStack.top());
                        containerStack.pop();
                        if (keyStack.empty()) {
                            _data = currentContainer;
                        } else {
                            Json currentJson(std::move(currentContainer));
                            std::visit(DataVisitor(currentJson, keyStack), containerStack.top());
                        }
                        if (moreDataShouldCome)
                            return;
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
                    case '{':
                        [[fallthrough]];
                    case '[':
                        [[fallthrough]];
                    case '"':
                        stateStack.push(ParseState::Value);
                        it--;
                        continue;
                    case ']': {
                        auto currentContainer = std::move(containerStack.top());
                        containerStack.pop();
                        if (keyStack.empty()) {
                            _data = currentContainer;
                        } else {
                            Json currentJson(std::move(currentContainer));
                            std::visit(DataVisitor(currentJson, keyStack), containerStack.top());
                        }
                        if (moreDataShouldCome)
                            return;
                        stateStack.pop();
                        continue;
                    }
                    default:
                        stateStack.push(ParseState::Value);
                        continue;
                }
            }
            case ParseState::Key: {
                moreDataShouldCome = false;
                switch(character) {
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
                        [[fallthrough]];
                    case ']':
                        if (!valueEnd)
                            valueEnd = it;
                        [[fallthrough]];
                    case ',': {
                        if (valueStart && !valueEnd)
                            valueEnd = it;
                        if (valueStart && valueEnd) {
                            std::string_view value(&*(*valueStart), std::distance(*valueStart, *valueEnd));

                            auto parsedValue = parseValue(value);
                            if(!parsedValue)
                                return;
                            std::visit(DataVisitor(*parsedValue, keyStack), containerStack.top());
                        }
                        stateStack.pop();
                        valueStart.reset();
                        valueEnd.reset();

                        if (character == ']' || character == '}')
                            it--;

                        moreDataShouldCome = character == ',';

                        continue;
                    }
                    case '{':
                        stateStack.push(ParseState::Object);
                        containerStack.emplace(std::map<std::string, Value>());
                        continue;
                    case '[':
                        stateStack.push(ParseState::Array);
                        containerStack.emplace(std::vector<Value>());
                        continue;
                    default:
                        if (!valueStart && std::isspace(character))
                            continue;
                        if (valueEnd && !std::isspace(character))
                            return;

                        if (valueStart && !valueEnd && std::isspace(character)) {
                            valueEnd = it;
                            continue;
                        }

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
                        continue;
                    default:
                        escaped = character == '\\';
                        break;
                }
                break;
            }
        }
    }

    _valid = true;
}

std::optional<Json> Json::getNode(const std::string &path) const {
    return {};
}

}