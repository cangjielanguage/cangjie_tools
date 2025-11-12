// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "SyscapCheck.cpp"
#include "SyscapCheck.h"
#include "Utils.h"

using namespace ark;

std::vector<uint8_t> StringToVector(const std::string &str)
{
    std::vector<uint8_t> input;
    for (char c : str) {
        input.push_back(static_cast<uint8_t>(c));
    }
    return input;
}

TEST(SyscapCheckTest, ParseJsonStringTest001)
{
    std::string str = "\"value\"";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "value");
}

TEST(SyscapCheckTest, ParseJsonStringTest002)
{
    std::string str = "\"value\"";
    auto input = StringToVector(str);
    size_t pos = 10;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "");
}

TEST(SyscapCheckTest, ParseJsonStringTest003)
{
    std::string str = "value\"";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "");
}

TEST(SyscapCheckTest, ParseJsonStringTest004)
{
    std::string str = "\"value";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "value");
}

TEST(SyscapCheckTest, ParseJsonNumberTest001)
{
    std::string str = "100";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 100);
}

TEST(SyscapCheckTest, ParseJsonNumberTest002)
{
    std::string str = "a00";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 0);
}

TEST(SyscapCheckTest, ParseJsonNumberTest003)
{
    std::string str = "10a";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 10);
}

TEST(SyscapCheckTest, ParseJsonNumberTest004)
{
    std::string str = "100";
    auto input = StringToVector(str);
    size_t pos = 10;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 0);
}

TEST(SyscapCheckTest, ParseJsonNumberTest005)
{
    std::string str = "*100";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 0);
}

TEST(SyscapCheckTest, ParseJsonArrayTest001)
{
    std::string str = "]";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto value = new JsonPair();
    ParseJsonArray(pos, input, value);
}

TEST(SyscapCheckTest, ParseJsonArrayTest002)
{
    std::string str = "[]";
    auto input = StringToVector(str);
    size_t pos = 10;
    auto value = new JsonPair();
    ParseJsonArray(pos, input, value);
}

TEST(SyscapCheckTest, ParseJsonArrayTest003)
{
    std::string str = "[\"array\" ";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto value = new JsonPair();
    ParseJsonArray(pos, input, value);
}

TEST(SyscapCheckTest, ParseJsonObjectTest001)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": 12a3b,
        "key4": {"subKey": "subValue"},
        "key5": {"subKey": {"subSubKey": "value"}},
        "key6": ["array", {"obj": "val"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj->pairs.size(), 6);
}

TEST(SyscapCheckTest, ParseJsonObjectTest002)
{
    std::string str = R"({
        "key1": "value1"
    })";
    auto input = StringToVector(str);
    size_t pos = 1000;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj, nullptr);
}

TEST(SyscapCheckTest, ParseJsonObjectTest003)
{
    std::string str = R"(
        "key1": "value1"
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj, nullptr);
}

TEST(SyscapCheckTest, ParseJsonObjectTest004)
{
    std::string str = R"({
        "key1": "value1"
    )";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj->pairs.size(), 1);
}

TEST(SyscapCheckTest, GetJsonStringTest001)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonString(obj, "subKey");

    EXPECT_EQ(res.size(), 1);
}

TEST(SyscapCheckTest, GetJsonStringTest002)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonString(obj, "key1");

    EXPECT_EQ(res.size(), 1);
}

TEST(SyscapCheckTest, GetJsonStringTest003)
{
    std::string str = R"({
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonString(obj, "key1");

    EXPECT_EQ(res.size(), 0);
}

TEST(SyscapCheckTest, GetJsonStringTest004)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonString(obj, "key");

    EXPECT_EQ(res.size(), 0);
}

TEST(SyscapCheckTest, GetJsonObjectTest001)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonObject(obj, "key1", 0);

    EXPECT_EQ(res, nullptr);
}

TEST(SyscapCheckTest, GetJsonObjectTest002)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonObject(obj, "key3", 0);

    EXPECT_NE(res, nullptr);
}

TEST(SyscapCheckTest, GetJsonObjectTest003)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": [{"obj1": "value1"}, {"obj2": "value2"}]}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonObject(obj, "obj", 1);

    EXPECT_NE(res, nullptr);
}

TEST(SyscapCheckTest, ParseJsonFileTest001)
{
    std::string json = R"({
        "Modules": {
            "module1": {
                "deviceSysCap": {"key1": "val1"}
            },
            "module2": {
                "deviceSysCap": {"key2": "val2"}
            }
        }
    })";
    std::vector<uint8_t> input = StringToVector(json);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap.size(), 2);
}

TEST(SyscapCheckTest, ParseJsonFileTest002)
{
    std::string json = R"({
        "Modules": {
            "module1": {
                "deviceSysCap": {"key1": "val1"}
            },
            "module2": {
                "deviceSysCap": {"key2": "val2"}
            }
        }
    })";
    std::vector<uint8_t> input = StringToVector(json);
    MessageHeaderEndOfLine::SetIsDeveco(true);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap.size(), 4);
}

TEST(SyscapCheckTest, ParseJsonFileTest003)
{
    std::string json = R"({
        "Modules": {
            "module1": "value1"
        }
    })";
    std::vector<uint8_t> input = StringToVector(json);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap.size(), 4);
}

TEST(SyscapCheckTest, ParseJsonFileTest004)
{
    std::string json = R"({
        "Modules": {}
    })";
    std::vector<uint8_t> input = StringToVector(json);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap.size(), 4);
}

TEST(SyscapCheckTest, CheckSysCapTest001)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    auto expr = new ArrayExpr();
    arg->expr = OwnedPtr<Expr>(expr);
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    auto hasAPILevel = true;
    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode, hasAPILevel);

    EXPECT_TRUE(result);
    EXPECT_TRUE(hasAPILevel);
}

TEST(SyscapCheckTest, CheckSysCapTest002)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    auto hasAPILevel = true;
    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode, hasAPILevel);

    EXPECT_TRUE(result);
    EXPECT_TRUE(hasAPILevel);
}

TEST(SyscapCheckTest, CheckSysCapTest003)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    auto expr = new LitConstExpr();
    expr->kind = LitConstKind::STRING;
    arg->expr = OwnedPtr<Expr>(expr);
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    auto hasAPILevel = true;
    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode, hasAPILevel);

    EXPECT_FALSE(result);
    EXPECT_TRUE(hasAPILevel);
}

TEST(SyscapCheckTest, CheckSysCapTest004)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    auto expr = new ArrayExpr();
    arg->expr = OwnedPtr<Expr>(expr);
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode);

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest005)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode);

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest006)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    auto expr = new LitConstExpr();
    expr->kind = LitConstKind::STRING;
    arg->expr = OwnedPtr<Expr>(expr);
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode);

    EXPECT_FALSE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest007)
{
    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(nullptr);

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest012)
{
    const std::string& syscapName = "syscap1";
    SyscapCheck syscapCheck;
    auto res = syscapCheck.CheckSysCap(syscapName);

    EXPECT_EQ(res, false);
}

TEST(SyscapCheckTest, ParseJsonArrayTest004)
{
    // Test parsing array with string values
    std::string str = "[\"value1\", \"value2\"]";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto value = MakeOwned<JsonPair>();
    ParseJsonArray(pos, input, value.get());

    EXPECT_EQ(value->valueStr.size(), 2);
    EXPECT_EQ(value->valueStr[0], "value1");
    EXPECT_EQ(value->valueStr[1], "value2");
}

TEST(SyscapCheckTest, ParseJsonArrayTest005)
{
    // Test parsing array with object values
    std::string str = "[{\"key\": \"value\"}]";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto value = MakeOwned<JsonPair>();
    ParseJsonArray(pos, input, value.get());

    EXPECT_EQ(value->valueObj.size(), 1);
    EXPECT_NE(value->valueObj[0], nullptr);
}

TEST(SyscapCheckTest, ParseJsonArrayTest006)
{
    // Test parsing empty array
    std::string str = "[]";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto value = MakeOwned<JsonPair>();
    ParseJsonArray(pos, input, value.get());

    EXPECT_TRUE(value->valueStr.empty());
    EXPECT_TRUE(value->valueObj.empty());
}

TEST(SyscapCheckTest, ParseJsonObjectTest005)
{
    // Test parsing object with number values
    std::string str = R"({"numberKey": 123})";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj->pairs.size(), 1);
    EXPECT_EQ(obj->pairs[0]->valueNum.size(), 1);
    EXPECT_EQ(obj->pairs[0]->valueNum[0], 123);
}

TEST(SyscapCheckTest, ParseJsonObjectTest006)
{
    // Test parsing nested objects
    std::string str = R"({"outer": {"inner": "value"}})";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj->pairs.size(), 1);
    EXPECT_EQ(obj->pairs[0]->valueObj.size(), 1);
    EXPECT_NE(obj->pairs[0]->valueObj[0], nullptr);
}

TEST(SyscapCheckTest, GetJsonObjectTest004)
{
    // Test getting nested JSON object
    std::string str = R"({
        "level1": {
            "level2": {
                "targetKey": "targetValue"
            }
        }
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonObject(obj, "level2", 0);

    EXPECT_NE(res, nullptr);
}

TEST(SyscapCheckTest, GetJsonObjectTest005)
{
    // Test getting non-existent JSON object
    std::string str = R"({"key": "value"})";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonObject(obj, "nonExistent", 0);

    EXPECT_EQ(res, nullptr);
}

TEST(SyscapCheckTest, ParseSyscapTest001)
{
    // Test parsing syscap from valid JSON structure
    auto deviceSysCapObj = MakeOwned<JsonObject>();
    auto pair = MakeOwned<JsonPair>();
    pair->key = "testKey";
    pair->valueStr.push_back("test/path/to/syscap.json");
    deviceSysCapObj->pairs.push_back(std::move(pair));

    auto result = ParseSyscap(deviceSysCapObj.get());

    // Since file reading will fail in test environment, expect empty set
    EXPECT_TRUE(result.empty());
}

TEST(SyscapCheckTest, SetIntersectionSetTest001)
{
    // Test setting intersection set for existing module
    SyscapCheck::module2SyscapsMap["testModule"] = {"syscap1", "syscap2"};

    SyscapCheck syscapCheck("testModule");
    syscapCheck.SetIntersectionSet("testModule");

    // Verify the set was properly initialized
    EXPECT_TRUE(syscapCheck.CheckSysCap("syscap1"));
    EXPECT_TRUE(syscapCheck.CheckSysCap("syscap2"));
}

TEST(SyscapCheckTest, SetIntersectionSetTest002)
{
    // Test setting intersection set for non-existent module
    SyscapCheck syscapCheck;
    syscapCheck.SetIntersectionSet("nonExistentModule");

    // Should not crash and should handle gracefully
    EXPECT_FALSE(syscapCheck.CheckSysCap("anySyscap"));
}

TEST(SyscapCheckTest, CheckSysCapTest008)
{
    // Test CheckSysCap with non-Decl node
    auto nonDeclNode = MakeOwned<Expr>();
    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(std::move(nonDeclNode));

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest009)
{
    // Test CheckSysCap with Decl containing non-APILevel annotation
    auto declNode = MakeOwned<Decl>();
    auto annotation = MakeOwned<Annotation>();
    annotation->identifier = "OtherAnnotation";
    declNode->annotations.push_back(std::move(annotation));

    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(std::move(declNode));

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest010)
{
    // Test CheckSysCap with APILevel annotation but non-syscap argument
    auto declNode = MakeOwned<Decl>();
    auto annotation = MakeOwned<Annotation>();
    annotation->identifier = "APILevel";
    auto arg = MakeOwned<FuncArg>();
    arg->name = "otherArg";
    annotation->args.push_back(std::move(arg));
    declNode->annotations.push_back(std::move(annotation));

    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(std::move(declNode));

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest013)
{
    // Test CheckSysCap with nullptr decl parameter
    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(static_cast<Ptr<Cangjie::AST::Decl>>(nullptr));

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest014)
{
    // Test CheckSysCap with valid syscap name
    SyscapCheck::module2SyscapsMap["testModule"] = {"testSyscap"};

    SyscapCheck syscapCheck("testModule");
    auto result = syscapCheck.CheckSysCap("testSyscap");

    EXPECT_TRUE(result);
}