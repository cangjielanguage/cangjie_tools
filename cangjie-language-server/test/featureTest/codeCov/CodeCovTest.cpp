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

namespace TestLspCodeCovTest {
    bool LspCodeCovTest(TestParam param)
    {
        SingleInstance *p = SingleInstance::GetInstance();
        std::string testFile = p->messagePath + "/" + param.testFile;

        std::string rootUri;
        bool isMultiModule = false;

        if (CreateMsg(p->pathIn, testFile, rootUri, isMultiModule) != true) {
            return false;
        }
        if (CreateBuildScript(p->pathBuildScript, testFile)) {
            BuildDynamicBinary(p->pathBuildScript);
        }
        /* Wait until the task is complete. The join blocking mode is not used. */
        StartLspServer(SingleInstance::GetInstance()->useDB);
        std::printf("curTestFile:\nfile:///%s\n", testFile.c_str());
        return true;
    }

    class CodeCovTest : public testing::TestWithParam<struct TestParam> {
    protected:
        void SetUp()
        {
            SetUpConfig("codeCov");
        }
    };

    INSTANTIATE_TEST_SUITE_P(CodeCov, CodeCovTest, testing::ValuesIn(GetTestCaseList("codeCov")));

    TEST_P(CodeCovTest, CodeCovCase)
    {
        TestParam param = GetParam();
        ASSERT_TRUE(LspCodeCovTest(param));
    }
}