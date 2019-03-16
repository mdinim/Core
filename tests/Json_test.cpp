//
// Created by Dániel Molnár on 2019-03-09.
//

#include <Json.hpp>
#include <TestUtil.hpp>

#include <gtest/gtest.h>

using namespace Core;

std::string emptyObject = "{}";

TEST(Json, canBeConstructed)
{
    Json json;
    ASSERT_TRUE(json);
}

TEST(Json, parsesValuesCorrectly)
{
    Json emptyArray("[]");
    ASSERT_TRUE(emptyArray);
    ASSERT_EQ(0, emptyArray.size());

    ASSERT_FALSE(emptyArray.get("[0]"));
    ASSERT_EQ(emptyArray.get("[0]", -1), -1);

}

TEST(Json, propertiesCanBeRetreived)
{
    using namespace std::string_literals;
    Json json("{ \"name\": \"Dani\", \"siblings\": [ {\"name\": \"Anna\", \"age\": 26}]}");

    ASSERT_TRUE(json.valid());
    ASSERT_TRUE(json.get("name"));
    auto firstSiblingName = json.get<std::string>("siblings[0].name");
    ASSERT_TRUE(firstSiblingName && firstSiblingName.value() == "Anna");
    auto age = json.get<int>("siblings[0].age");
    ASSERT_TRUE(age && age.value() == 26);
}