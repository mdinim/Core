//
// Created by Dániel Molnár on 2019-03-09.
//

#include <Json/Json.hpp>
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
                          "  \"phone_number[alternative]\": \"838 919-1212\",\n"
                          "  \"gender\": {\n"
                          "    \"type\": \"male\"\n"
                          "  },\n"
                          "  \"this_key_is_odd[0]\": \"really_odd\","
                          "  \"this_key_is_odd\": [ null, false ]\n"
                          "}";

TEST(Json, can_be_constructed)
{
    Json json = Json::create_object();
    ASSERT_TRUE(json);
    ASSERT_EQ(json, Json("{}"));
    EXPECT_NE(json, Json("[]"));
}

TEST(Json, values_parsed_correctly)
{
    Json json(sample_json);
    ASSERT_TRUE(json);
    EXPECT_EQ(json.size(), 9);
    EXPECT_EQ(json, Json(sample_json));
    std::stringstream stream;
    stream << json;
    EXPECT_EQ(json, Json(stream.str()));

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

TEST(Json, exceptions)
{
    Json array = Json::create_array();
    Json object = Json::create_object();
    try {
        array.at("invalid");
    } catch(Core::bad_json_access& ex) {
        EXPECT_FALSE(std::string(ex.what()).empty());
    }
    
    try {
        array.set("invalid", Json::Null());
    } catch(Core::bad_json_access& ex) {
        EXPECT_FALSE(std::string(ex.what()).empty());
    }
    
    try {
        object.set("[0]", Json::Null());
    } catch(Core::bad_json_access& ex) {
        EXPECT_FALSE(std::string(ex.what()).empty());
    }
    
    try {
        array.set("[", Json::Null());
    } catch (Core::bad_json_path& ex) {
        EXPECT_FALSE(std::string(ex.what()).empty());
    }
}

TEST(Json, path_access)
{
    const Json const_json(sample_json);
    Json json(sample_json);
    
    const Json const_array("[1, 2, 3, 4, 5, 6, 7, 8]");
    Json array("[1, 2, 3, 4, 5, 6, 7, 8]");
    
    Json long_array = Json::create_array();
    EXPECT_THROW(long_array.set("[999999999999999]", 0), Core::bad_json_path);
    EXPECT_THROW(long_array.set("", 0), Core::bad_json_path);
    
    ASSERT_TRUE(json && const_json && array && const_array);

    auto first_phone_number = json.get<std::string>("phone_number[0].number");
    EXPECT_EQ(*first_phone_number, "212 555-1234");
    auto second_phone_number = json.get<std::string>("phone_number[1].number");
    EXPECT_EQ(*second_phone_number, "646 555-4567");
    auto path_with_index_like_elements = json.get<std::string>("phone_number[alternative]");
    EXPECT_EQ(*path_with_index_like_elements, "838 919-1212");
    
    // object accessed as array
    EXPECT_THROW(json.at(0), Core::bad_json_access);
    EXPECT_THROW(json[0], Core::bad_json_access);
    // correct access
    EXPECT_NO_THROW(json["phone_number"]);
    EXPECT_NO_THROW(json.at("phone_number"));
    // array accessed as object
    EXPECT_THROW(array.at("key"), Core::bad_json_access);
    EXPECT_THROW(array["key"], Core::bad_json_access);
    // correct access
    EXPECT_NO_THROW(array[0]);
    EXPECT_NO_THROW(array.at(0));
    // out of bounds
    EXPECT_THROW(array.at(99), std::out_of_range);
    
    // const object accessed as array
    EXPECT_THROW(const_json.at(0), Core::bad_json_access);
    EXPECT_THROW(const_json[0], Core::bad_json_access);
    // correct access
    EXPECT_NO_THROW(const_json.at("phone_number"));
    // const array accessed as object
    EXPECT_THROW(const_array.at("key"), Core::bad_json_access);
    // correct access
    EXPECT_NO_THROW(const_array[0]);
    EXPECT_NO_THROW(const_array.at(0));
    // out of bounds
    EXPECT_THROW(const_array.at(99), std::out_of_range);
    
    // type-specific functions
    EXPECT_THROW(json.push_back(Json::Null()), Core::bad_json_access);
    EXPECT_THROW(json.pop_back(), Core::bad_json_access);
    
    
    EXPECT_EQ(json.get<std::string>("this_key_is_odd\\[0\\]", "not_odd"), "really_odd");
    EXPECT_EQ(json.get<Json::Null>("this_key_is_odd[0]"), Json::Null());
    EXPECT_FALSE(json.get<Json::Null>(""));
    
    // non-existing paths
    EXPECT_FALSE(json.get<std::string>("[0]"));
    EXPECT_FALSE(json.get<Json>("phone_number")->get<std::string>("not_an_object"));
    EXPECT_FALSE(json.get<std::string>("non_existing_key"));
    EXPECT_FALSE(json.get<std::string>("phone_number[99]"));
}

TEST(Json, array_in_array)
{
    auto json = Json::create_array();
    json.set("[0][0].dummy", std::string("what"));
    json.set("[2][1]", 12.2412);
    std::stringstream stuff;
    stuff << json << std::endl;
    Json reparsed(stuff.str());
    ASSERT_TRUE(reparsed.valid());
    ASSERT_EQ(reparsed, json);
    ASSERT_EQ(*reparsed.get<std::string>("[0][0].dummy"), "what");
    
    Json empty_array("[]");
    ASSERT_TRUE(empty_array);
}

TEST(Json, value_test)
{
    auto max_long_long = std::to_string(std::numeric_limits<long long>::max());
    TEST_INFO << "Trying maximum value: " << max_long_long << std::endl;
    Json special_values("[true, false, null, -12.2e34, 15e+3, 1251.151258129, 325.12e13 , 0.124, 0e-15,"
                        + max_long_long +", \" space_in_front\"]");
    ASSERT_TRUE(special_values);
    ASSERT_EQ(std::get<bool>(special_values.at(0)), true);
    ASSERT_EQ(std::get<std::string>(special_values.at(10)), " space_in_front");
    std::vector<Json> invalid_special_values{
        {"[0 .e]"},
        {"[0o]"},
        {"[folse]"},
        {"[treu]"},
        {"[12.l]"},
        {"[45. ]"},
        {"[42.32e+]"},
        {"[42.32el]"},
        {"[\"unclosed_string]"},
        {"[\"wrong_escape_sequence\\l\"]"},
        {"[" + max_long_long + "9]"}
    };
    
    for (const auto& json : invalid_special_values) {
        ASSERT_FALSE(json);
    }
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
    EXPECT_EQ(new_json.size(), 0);
    new_json["hello"] = "world"s;
    EXPECT_EQ(new_json.size(), 1);
    new_json["a_number"] = 2;
    EXPECT_EQ(new_json.size(), 2);
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
    
    Json array("\n\r\t\n\n [{\"\\n\":null}]"s);
    ASSERT_TRUE(array);
}

TEST(Json, escaped_characters)
{
    Json object("{ \"unicode\\n\\u8484 s\": \"asd\"}"s);
    std::stringstream object_string;
    object_string << object;
    ASSERT_TRUE(object);
    Json object_copy(object_string.str());
    ASSERT_EQ(object_copy, object);
    Json invalid_object("{\"\\u3k62\": \"o\"}"s);
    ASSERT_FALSE(invalid_object);
    Json second_invalid_object("{\"\\l\":\"o\"}"s);
    ASSERT_FALSE(second_invalid_object);
}

TEST(Json, coma_error)
{
    Json array("[ \"this\",\"is\",\"fun\",]");
    ASSERT_FALSE(array);
}

TEST(Json, invalid_inner_data)
{
    std::vector<Json> invalid_inner_objects{
        {"{\"data\": [[]}"s},
        {"{\"data\": [[}]]}"s},
        {"[{\"hello\": \"\"]]"s},
        {"[{null}]"s},
        {"{{]}"s},
        {"{\"data\": {null]}"s},
        {"[{\"data\": nall}]"s},
        {"[nall]"s}
    };
    
    for(const auto& json : invalid_inner_objects) {
        ASSERT_FALSE(json);
    }
}

TEST(Json, garbage_at_the_end)
{
    Json invalid_object("{}garbage");
    ASSERT_FALSE(invalid_object);
    Json invalid_array("[]garbage");
    ASSERT_FALSE(invalid_array);
}

TEST(Json, double_colon)
{
    Json json("{\"hello\"  \n: \"world\"}");
    ASSERT_TRUE(json);
    Json missing("{\"hello\" null}");
    ASSERT_FALSE(missing);
}
