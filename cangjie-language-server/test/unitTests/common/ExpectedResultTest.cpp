// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "common.h"
#include "SingleInstance.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

namespace {
class ExpectedResultTest : public testing::Test {
protected:
    void SetUp() override
    {
        static std::atomic<unsigned> next{0};
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        root = std::filesystem::temp_directory_path() /
            ("cangjie-lsp-expected-" + std::to_string(stamp) + "-" + std::to_string(next++));
        std::filesystem::create_directories(root);
        previousMessagePath = SingleInstance::GetInstance()->messagePath;
        SingleInstance::GetInstance()->messagePath = root.string();
    }

    void TearDown() override
    {
        SingleInstance::GetInstance()->messagePath = previousMessagePath;
        std::error_code error;
        std::filesystem::remove_all(root, error);
    }

    nlohmann::json Read(const std::string& content)
    {
        const auto path = root / "result.base";
        {
            std::ofstream file(path);
            file << content;
        }
        std::string baseFile = "result.base";
        return test::common::ReadExpectedResult(baseFile);
    }

    std::filesystem::path root;
    std::string previousMessagePath;
};
} // namespace

TEST_F(ExpectedResultTest, RejectsTruncatedJsonAndStillReadsTheNextCase)
{
    nlohmann::json invalid;
    EXPECT_NONFATAL_FAILURE(invalid = Read("{\"result\":[{\"docum[       OK ] unrelated test\n"),
        "Invalid expected JSON object");
    EXPECT_TRUE(invalid.empty());
    nlohmann::json actual = {{"id", 9}, {"jsonrpc", "2.0"}, {"result", nlohmann::json::array()}};
    std::string reason;
    EXPECT_FALSE(test::common::CheckResult(invalid, actual, TestType::Completion, reason));
    EXPECT_EQ(Read(actual.dump()), actual);
}

TEST_F(ExpectedResultTest, DoesNotSkipCorruptFirstRecordToAcceptLaterOutput)
{
    EXPECT_NONFATAL_FAILURE(EXPECT_TRUE(Read("{broken\n{\"id\":9,\"result\":[]}\n").empty()),
        "Invalid expected JSON object");
}

TEST_F(ExpectedResultTest, AcceptsBlankPrefixAndValidNullResult)
{
    const nlohmann::json expected = {{"id", 9}, {"jsonrpc", "2.0"}, {"result", nullptr}};
    EXPECT_EQ(Read("\n \t\r\n" + expected.dump() + "\n"), expected);
}

TEST_F(ExpectedResultTest, RejectsEmptyOrNonObjectExpectedData)
{
    EXPECT_NONFATAL_FAILURE(EXPECT_TRUE(Read(" \r\n").empty()), "Expected result is empty");
    EXPECT_NONFATAL_FAILURE(EXPECT_TRUE(Read("[]\n").empty()), "Invalid expected JSON object");
    EXPECT_NONFATAL_FAILURE(EXPECT_TRUE(Read("null\n").empty()), "Invalid expected JSON object");
}

TEST_F(ExpectedResultTest, MissingExpectedFileRecordsFailure)
{
    std::string missing = "missing.base";
    EXPECT_NONFATAL_FAILURE(test::common::ReadExpectedResult(missing), "Cannot open expected result");
}
