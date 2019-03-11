//
// Created by Dániel Molnár on 2019-03-09.
//

#include <Json.hpp>
#include <TestUtil.hpp>

#include <gtest/gtest.h>

using namespace Core;

const std::string testJson =
"{"
"   \"firstName\": \"John\","
"   \"lastName\": \"Doe\","
"   \"age\": 2,"
"   \"gender\": \"male\","
"   \"sister\": {"
"       \"name\": \"Lily\","
"       \"age\": 12"
"   },"
"   \"backpack\": ["
"       {"
"           \"id\": \"apple\""
"       },"
"       {"
"           \"id\": \"pear\""
"       },"
"       \"asd\","
"       ["
"           \"a\","
"           \"b\""
"       ]"
"   ],"
"   \"married\": false,"
"   \"snackpack\": {\"A\": \"C\"},"
"   \"hello\": \"world\","
"   \"exponentNumber\": 10.3E-162"
"}";

TEST(Json, canBeConstructed)
{

}

TEST(Json, parsesValuesCorrectly)
{
    Json json(testJson);
    ASSERT_TRUE(json.valid());
}