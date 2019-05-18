//
// Created by Dániel Molnár on 2019-03-09.
//

#include <Json.hpp>
#include <Utils/TestUtil.hpp>

#include <gtest/gtest.h>

using namespace Core;
using namespace std::string_literals;

std::string sampleJson = "{\n"
                          "  \"firstName\": \"John\",\n"
                          "  \"lastName\": \"Smith\",\n"
                          "  \"age\": 25,\n"
                          "  \"address\": {\n"
                          "    \"streetAddress\": \"21 2nd Street\",\n"
                          "    \"city\": \"New York\",\n"
                          "    \"state\": \"NY\",\n"
                          "    \"postalCode\": 10021\n"
                          "  },\n"
                          "  \"phoneNumber\": [\n"
                          "    {\n"
                          "      \"type\": \"home\",\n"
                          "      \"number\": \"212 555-1234\"\n"
                          "    },\n"
                          "    {\n"
                          "      \"type\": \"fax\",\n"
                          "      \"number\": \"646 555-4567\"\n"
                          "    }\n"
                          "  ],\n"
                          "  \"gender\": {\n"
                          "    \"type\": \"male\"\n"
                          "  }\n"
                          "}";

TEST(Json, canBeConstructed)
{
    Json json = Json::createObject();
    ASSERT_TRUE(json);
    ASSERT_EQ(json, Json("{}"));
}

TEST(Json, parsesValuesCorrectly)
{
    Json json(sampleJson);
    ASSERT_TRUE(json);

    auto firstName = json.get<std::string>("firstName");
    ASSERT_TRUE(firstName);
    EXPECT_EQ(*firstName, "John");

    auto lastName = json.get<std::string>("lastName");
    ASSERT_TRUE(lastName);
    EXPECT_EQ(*lastName, "Smith");

    EXPECT_FALSE(json.get<int>("firstName"));

    long age = json.get("age", 0);
    EXPECT_EQ(age, 25);

    auto address = json.get<Json>("address");
    ASSERT_TRUE(address);
    EXPECT_EQ(*address, Json("{\n"
                            "  \"streetAddress\": \"21 2nd Street\",\n"
                            "  \"city\": \"New York\",\n"
                            "  \"state\": \"NY\",\n"
                            "  \"postalCode\": 10021\n"
                            "}"));
}

TEST(Json, pathAccess)
{
    Json json(sampleJson);
    ASSERT_TRUE(json);

    auto firstPhoneNumber = json.get<std::string>("phoneNumber[0].number");
    ASSERT_TRUE(firstPhoneNumber);
    EXPECT_EQ(*firstPhoneNumber, "212 555-1234");
    auto secondPhoneNumber = json.get<std::string>("phoneNumber[1].number");
    EXPECT_EQ(*secondPhoneNumber, "646 555-4567");

}

TEST(Json, propertiesCanBeAccessedAndModified)
{
    Json json(sampleJson);
    ASSERT_TRUE(json);

    EXPECT_THROW(json[0], bad_json_access);
    auto& value = json["firstName"];
    EXPECT_EQ("John"s, value);
    value = "Jake"s;
    EXPECT_EQ(value, "Jake"s);
    EXPECT_EQ(json.get<std::string>("firstName").value(), "Jake"s);

    auto newJson = Json::createObject();
    newJson["hello"] = "world"s;
    newJson["aNumber"] = 2;
    EXPECT_EQ(newJson, Json("{\"hello\": \"world\", \"aNumber\": 2}"));
}

TEST(Json, arrayAccess)
{
    auto json = Json::createArray();

    EXPECT_THROW(json.at(2), std::out_of_range);
    json.push_back(std::string("Stuff"));
    EXPECT_EQ(json.at(0), std::string("Stuff"));
    json.pop_back();

    json.set("[5]", std::string("Hello"));
    for(auto i = 0u; i < 5u; ++i) {
        EXPECT_EQ(json[i], Json::Null());
    }
}