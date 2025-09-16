// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"

#include<string>
#include<thread>
#include<vector>

#include "common.h"
#include "SingleInstance.h"

using namespace test::common;

namespace TestBreakpoints {
    bool LspBreakpointsTest(TestParam param)
    {
        SingleInstance *p = SingleInstance::GetInstance();
        std::string testFile = p->messagePath + "/" + param.testFile;

        std::string rootUri;
        bool isMultiModule = false;

        if (CreateMsg(p->pathIn, testFile, rootUri, isMultiModule) != true) {
            return false;
        }
        if (IsMacroExpandTest(rootUri)) {
            return true;
        }
        if (CreateBuildScript(p->pathBuildScript, testFile)) {
            BuildDynamicBinary(p->pathBuildScript);
        }
        /* Wait until the task is complete. The join blocking mode is not used. */
        StartLspServer(SingleInstance::GetInstance()->useDB);

        /* Check the test case result. */
        std::vector<ark::BreakpointLocation> expect = ReadExpectedBreakpointLocationItems(param.baseFile);
        std::vector<ark::BreakpointLocation> actual = CreateBreakpointStruct(ReadFileById(p->pathOut, param.id));

        /* if case is diff show info */
        std::string reason;
        bool showErr = CheckBreakpointResult(expect, actual, reason);
        if (!showErr) {
            nlohmann::json expLines = ReadExpectedResult(param.baseFile);
            nlohmann::json result = ReadFileById(p->pathOut, param.id);
            std::cout << "the false reason is : " << reason << std::endl;
            ShowDiff(expLines, result, param, p->messagePath);
        }
        return showErr;
    }

    class BreakpointsTest : public testing::TestWithParam<struct TestParam> {
    protected:
        void SetUp()
        {
            SetUpConfig("breakpoints");
        }
    };

    INSTANTIATE_TEST_SUITE_P(Breakpoints, BreakpointsTest, testing::ValuesIn(GetTestCaseList("breakpoints")));

    TEST_P(BreakpointsTest, BreakpointCase)
    {
        TestParam param = GetParam();
        ASSERT_TRUE(LspBreakpointsTest(param));
    }
}