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


// These production settings are process-wide. Keep every case independent of
// suite order, other test suites, and --gtest_repeat.
class SyscapCheckTest : public testing::Test {
protected:
    void SetUp() override
    {
        previousModules = std::move(SyscapCheck::module2SyscapsMap);
        previousChecked = SyscapCheck::isChecked;
        previousDeveco = MessageHeaderEndOfLine::GetIsDeveco();
        SyscapCheck::module2SyscapsMap.clear();
        SyscapCheck::isChecked = true;
        MessageHeaderEndOfLine::SetIsDeveco(true);
    }

    void TearDown() override
    {
        SyscapCheck::module2SyscapsMap = std::move(previousModules);
        SyscapCheck::isChecked = previousChecked;
        MessageHeaderEndOfLine::SetIsDeveco(previousDeveco);
    }

    static void SeedExistingModules()
    {
        SyscapCheck::module2SyscapsMap = {
            {"module1", {}}, {"module2", {}},
            {"ohos_app_cangjie_module1", {}}, {"ohos_app_cangjie_module2", {}}
        };
    }

private:
    std::unordered_map<std::string, SysCapSet> previousModules;
    bool previousChecked = false;
    bool previousDeveco = false;
};

std::vector<uint8_t> StringToVector(const std::string &str)
{
    std::vector<uint8_t> input;
    for (char c : str) {
        input.push_back(static_cast<uint8_t>(c));
    }
    return input;
}

TEST_F(SyscapCheckTest, ParseJsonStringTest001)
{
    std::string str = "\"value\"";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "value");
}

TEST_F(SyscapCheckTest, ParseJsonStringTest002)
{
    std::string str = "\"value\"";
    auto input = StringToVector(str);
    size_t pos = 10;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "");
}

TEST_F(SyscapCheckTest, ParseJsonStringTest003)
{
    std::string str = "value\"";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "");
}

TEST_F(SyscapCheckTest, ParseJsonStringTest004)
{
    std::string str = "\"value";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "value");
}

TEST_F(SyscapCheckTest, ParseJsonNumberTest001)
{
    std::string str = "100";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 100);
}

TEST_F(SyscapCheckTest, ParseJsonNumberTest002)
{
    std::string str = "a00";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 0);
}

TEST_F(SyscapCheckTest, ParseJsonNumberTest003)
{
    std::string str = "10a";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 10);
}

TEST_F(SyscapCheckTest, ParseJsonNumberTest004)
{
    std::string str = "100";
    auto input = StringToVector(str);
    size_t pos = 10;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 0);
}

TEST_F(SyscapCheckTest, ParseJsonNumberTest005)
{
    std::string str = "*100";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 0);
}

TEST_F(SyscapCheckTest, ParseJsonArrayTest001)
{
    std::string str = "]";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto value = new JsonPair();
    ParseJsonArray(pos, input, value);
}

TEST_F(SyscapCheckTest, ParseJsonArrayTest002)
{
    std::string str = "[]";
    auto input = StringToVector(str);
    size_t pos = 10;
    auto value = new JsonPair();
    ParseJsonArray(pos, input, value);
}

TEST_F(SyscapCheckTest, ParseJsonArrayTest003)
{
    std::string str = "[\"array\" ";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto value = new JsonPair();
    ParseJsonArray(pos, input, value);
}

TEST_F(SyscapCheckTest, ParseJsonObjectTest001)
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

TEST_F(SyscapCheckTest, ParseJsonObjectTest002)
{
    std::string str = R"({
        "key1": "value1"
    })";
    auto input = StringToVector(str);
    size_t pos = 1000;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj, nullptr);
}

TEST_F(SyscapCheckTest, ParseJsonObjectTest003)
{
    std::string str = R"(
        "key1": "value1"
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj, nullptr);
}

TEST_F(SyscapCheckTest, ParseJsonObjectTest004)
{
    std::string str = R"({
        "key1": "value1"
    )";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj->pairs.size(), 1);
}

TEST_F(SyscapCheckTest, GetJsonStringTest001)
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

TEST_F(SyscapCheckTest, GetJsonStringTest002)
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

TEST_F(SyscapCheckTest, GetJsonStringTest003)
{
    std::string str = R"({
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonString(obj, "key1");

    EXPECT_EQ(res.size(), 0);
}

TEST_F(SyscapCheckTest, GetJsonStringTest004)
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

TEST_F(SyscapCheckTest, GetJsonObjectTest001)
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

TEST_F(SyscapCheckTest, GetJsonObjectTest002)
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

TEST_F(SyscapCheckTest, GetJsonObjectTest003)
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

TEST_F(SyscapCheckTest, ParseJsonFileTest001)
{
    MessageHeaderEndOfLine::SetIsDeveco(false);
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
    EXPECT_EQ(syscapCheck.module2SyscapsMap.count("module1"), 1U);
    EXPECT_EQ(syscapCheck.module2SyscapsMap.count("module2"), 1U);
}

TEST_F(SyscapCheckTest, ParseJsonFileTest002)
{
    // Parsing in DevEco mode must add prefixed keys without replacing existing plain keys.
    SyscapCheck::module2SyscapsMap = {{"module1", {}}, {"module2", {}}};
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
    EXPECT_EQ(syscapCheck.module2SyscapsMap.count("ohos_app_cangjie_module1"), 1U);
    EXPECT_EQ(syscapCheck.module2SyscapsMap.count("ohos_app_cangjie_module2"), 1U);
}

TEST_F(SyscapCheckTest, ParseJsonFileTest003)
{
    SeedExistingModules();
    const auto before = SyscapCheck::module2SyscapsMap;
    std::string json = R"({
        "Modules": {
            "module1": "value1"
        }
    })";
    std::vector<uint8_t> input = StringToVector(json);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap, before);
}

TEST_F(SyscapCheckTest, ParseJsonFileTest004)
{
    SeedExistingModules();
    const auto before = SyscapCheck::module2SyscapsMap;
    std::string json = R"({
        "Modules": {}
    })";
    std::vector<uint8_t> input = StringToVector(json);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap, before);
}

TEST_F(SyscapCheckTest, CheckSysCapTest001)
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

TEST_F(SyscapCheckTest, CheckSysCapTest002)
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

TEST_F(SyscapCheckTest, CheckSysCapTest003)
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
    SyscapCheck::isChecked = true;
    auto result = syscapCheck.CheckSysCap(declNode, hasAPILevel);

    EXPECT_FALSE(result);
    EXPECT_TRUE(hasAPILevel);
}

TEST_F(SyscapCheckTest, CheckSysCapTest004)
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

TEST_F(SyscapCheckTest, CheckSysCapTest005)
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

TEST_F(SyscapCheckTest, CheckSysCapTest006)
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
    SyscapCheck::isChecked = true;
    auto result = syscapCheck.CheckSysCap(declNode);

    EXPECT_FALSE(result);
}

TEST_F(SyscapCheckTest, CheckSysCapTest007)
{
    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(nullptr);

    EXPECT_TRUE(result);
}

TEST_F(SyscapCheckTest, CheckSysCapTest012)
{
    const std::string& syscapName = "syscap1";
    SyscapCheck syscapCheck;
    SyscapCheck::isChecked = true;
    auto res = syscapCheck.CheckSysCap(syscapName);

    EXPECT_EQ(res, false);
}

TEST_F(SyscapCheckTest, ParseJsonArrayTest004)
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

TEST_F(SyscapCheckTest, ParseJsonArrayTest005)
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

TEST_F(SyscapCheckTest, ParseJsonArrayTest006)
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

TEST_F(SyscapCheckTest, ParseJsonObjectTest005)
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

TEST_F(SyscapCheckTest, ParseJsonObjectTest006)
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

TEST_F(SyscapCheckTest, GetJsonObjectTest004)
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

TEST_F(SyscapCheckTest, GetJsonObjectTest005)
{
    // Test getting non-existent JSON object
    std::string str = R"({"key": "value"})";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonObject(obj, "nonExistent", 0);

    EXPECT_EQ(res, nullptr);
}

TEST_F(SyscapCheckTest, ParseSyscapTest001)
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

TEST_F(SyscapCheckTest, SetIntersectionSetTest001)
{
    // Test setting intersection set for existing module
    SyscapCheck::module2SyscapsMap["testModule"] = {"syscap1", "syscap2"};

    SyscapCheck syscapCheck("testModule");
    syscapCheck.SetIntersectionSet("testModule");

    // Verify the set was properly initialized
    EXPECT_TRUE(syscapCheck.CheckSysCap("syscap1"));
    EXPECT_TRUE(syscapCheck.CheckSysCap("syscap2"));
}

TEST_F(SyscapCheckTest, SetIntersectionSetTest002)
{
    // Test setting intersection set for non-existent module
    SyscapCheck syscapCheck;
    SyscapCheck::isChecked = true;
    syscapCheck.SetIntersectionSet("nonExistentModule");

    // Should not crash and should handle gracefully
    EXPECT_FALSE(syscapCheck.CheckSysCap("anySyscap"));
}

TEST_F(SyscapCheckTest, CheckSysCapTest008)
{
    // Test CheckSysCap with non-Decl node
    auto nonDeclNode = MakeOwned<Expr>();
    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(std::move(nonDeclNode));

    EXPECT_TRUE(result);
}

TEST_F(SyscapCheckTest, CheckSysCapTest009)
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

TEST_F(SyscapCheckTest, CheckSysCapTest010)
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

TEST_F(SyscapCheckTest, CheckSysCapTest013)
{
    // Test CheckSysCap with nullptr decl parameter
    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(static_cast<Ptr<Cangjie::AST::Decl>>(nullptr));

    EXPECT_TRUE(result);
}

TEST_F(SyscapCheckTest, CheckSysCapTest014)
{
    // Test CheckSysCap with valid syscap name
    SyscapCheck::module2SyscapsMap["testModule"] = {"testSyscap"};

    SyscapCheck syscapCheck("testModule");
    auto result = syscapCheck.CheckSysCap("testSyscap");

    EXPECT_TRUE(result);
}