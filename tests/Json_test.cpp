//
// Created by Dániel Molnár on 2019-03-09.
//

#include <Json.hpp>
#include <Utils/TestUtil.hpp>

#include <gtest/gtest.h>

using namespace Core;
using namespace std::string_literals;

std::string sample_json = "{\n"
                          "  \"first_name\": \"John\",\n"
                          "  \"last_name\": \"Smith\",\n"
                          "  \"age\": 25,\n"
                          "  \"address\": {\n"
                          "    \"street_address\": \"21 2nd Street\",\n"
                          "    \"city\": \"New York\",\n"
                          "    \"state\": \"NY\",\n"
                          "    \"postal_code\": 10021\n"
                          "  },\n"
                          "  \"phone_number\": [\n"
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

TEST(Json, can_be_constructed)
{
    Json json = Json::create_object();
    ASSERT_TRUE(json);
    ASSERT_EQ(json, Json("{}"));
}

TEST(Json, values_parsed_correctly)
{
    Json json(sample_json);
    ASSERT_TRUE(json);

    auto first_name = json.get<std::string>("first_name");
    ASSERT_TRUE(first_name);
    EXPECT_EQ(*first_name, "John");

    auto last_name = json.get<std::string>("last_name");
    ASSERT_TRUE(last_name);
    EXPECT_EQ(*last_name, "Smith");

    EXPECT_FALSE(json.get<int>("first_name"));

    long age = json.get("age", 0);
    EXPECT_EQ(age, 25);

    auto address = json.get<Json>("address");
    ASSERT_TRUE(address);
    EXPECT_EQ(*address, Json("{\n"
                            "  \"street_address\": \"21 2nd Street\",\n"
                            "  \"city\": \"New York\",\n"
                            "  \"state\": \"NY\",\n"
                            "  \"postal_code\": 10021\n"
                            "}"));
}

TEST(Json, path_access)
{
    Json json(sample_json);
    ASSERT_TRUE(json);

    auto first_phone_number = json.get<std::string>("phone_number[0].number");
    ASSERT_TRUE(first_phone_number);
    EXPECT_EQ(*first_phone_number, "212 555-1234");
    auto second_phone_number = json.get<std::string>("phone_number[1].number");
    EXPECT_EQ(*second_phone_number, "646 555-4567");

}

TEST(Json, array_in_array)
{
    auto json = Json::create_array();
    json.set("[0][0].dummy", std::string("what"));
    std::stringstream stuff;
    stuff << json << std::endl;
    std::cout << stuff.str() << std::endl;
    Json reparsed(stuff.str());
    std::cout << reparsed << std::endl;
    ASSERT_TRUE(reparsed.valid());
    
    ASSERT_EQ(*reparsed.get<std::string>("[0][0].dummy"), "what");
}

TEST(Json,  property_access_and_modification)
{
    Json json(sample_json);
    ASSERT_TRUE(json);

    EXPECT_THROW(json[0], bad_json_access);
    auto& value = json["first_name"];
    EXPECT_EQ("John"s, value);
    value = "Jake"s;
    EXPECT_EQ(value, "Jake"s);
    EXPECT_EQ(json.get<std::string>("first_name").value(), "Jake"s);

    auto new_json = Json::create_object();
    new_json["hello"] = "world"s;
    new_json["a_number"] = 2;
    EXPECT_EQ(new_json, Json("{\"hello\": \"world\", \"a_number\": 2}"));
}

TEST(Json, array_access)
{
    auto json = Json::create_array();

    EXPECT_THROW(json.at(2), std::out_of_range);
    json.push_back(std::string("Stuff"));
    EXPECT_EQ(json.at(0), std::string("Stuff"));
    json.pop_back();

    json.set("[5]", std::string("Hello"));
    for(auto i = 0u; i < 5u; ++i) {
        EXPECT_EQ(json[i], Json::Null());
    }
}

TEST(Json, leading_white_spaces)
{
    Json object("\n\t\r   {}"s);
    ASSERT_TRUE(object);
    
    Json array("\n\r\t\n\n [{}]"s);
    ASSERT_TRUE(array);
}

TEST(Json, escaped_characters)
{
    Json object("{ \"line_broken_\n\r\f\b\\\"key\\\"\": \"value\", \"unicode\\u8484 s\": \"\u1234\"}"s);
    ASSERT_TRUE(object);
}
