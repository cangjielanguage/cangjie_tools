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

namespace TestLspSemanticHighlight {
    bool LspSemanticHighlightTest(TestParam param)
    {
        SingleInstance *p = SingleInstance::GetInstance();
        std::string testFile = p->messagePath + "/" + param.testFile;
        std::string baseFile = p->messagePath + "/" + param.baseFile;

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
        nlohmann::json expLines = ReadExpectedResult(param.baseFile);
        nlohmann::json result = ReadFileById(p->pathOut, param.id);

        /* if CheckResult return false, print the reason */
        std::string info = "none";
        /* if case is diff show info */
        bool showErr = CheckResult(expLines, result, TestType::SemanticHighlight, info);
        if (!showErr) {
            std::cout << "the false reason is : " << info << std::endl;
            ShowDiff(expLines, result, param, p->messagePath);
        }
        return showErr;
    }

    class SemanticHighlightTest : public testing::TestWithParam<struct TestParam> {
    protected:
        void SetUp()
        {
            SetUpConfig("semanticHighlight");
        }
    };

    INSTANTIATE_TEST_SUITE_P(SemanticHighlight, SemanticHighlightTest, testing::ValuesIn(GetTestCaseList("semanticHighlight")));

    TEST_P(SemanticHighlightTest, SemanticHighlightCase)
    {
        TestParam param = GetParam();
        ASSERT_TRUE(LspSemanticHighlightTest(param));
    }
}