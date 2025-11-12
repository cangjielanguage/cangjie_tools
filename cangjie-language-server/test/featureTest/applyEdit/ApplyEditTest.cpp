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

namespace TestLspApplyEditTest {
    bool LspApplyEditTest(TestParam param)
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
        std::thread ThreadObj(StartLspServer, SingleInstance::GetInstance()->useDB);
        ThreadObj.join();

        /* Check the test case result. */
        nlohmann::json expLines = ReadExpectedResult(param.baseFile);
        ChangeApplyEditUrlForBaseFile(testFile, expLines, rootUri, isMultiModule);
        nlohmann::json result = ReadFileByMethod(p->pathOut, param.method);

        /* if case is diff show info */
        std::string info = "none";
        bool showErr = TestUtils::CheckApplyEditResult(expLines, result, info);
        if (!showErr) {
            std::cout << "the false reason is : " << info << std::endl;
            ShowDiff(expLines, result, param, p->messagePath);
        }
        return true;
    }

    class ApplyEditTest : public testing::TestWithParam<struct TestParam> {
    protected:
        void SetUp()
        {
            SingleInstance *p = SingleInstance::GetInstance();
            p->testFolder = "applyEdit";
            p->pathIn = GetRealPath(p->testFolder + "_freopen.in");
            p->pathOut = GetRealPath(p->testFolder + "_freopen.out");
            p->pathPwd = GetPwd();
            p->workPath = GetRootPath(p->pathPwd);
            p->messagePath = p->workPath + "/test/message/" + p->testFolder;
        }
    };
    INSTANTIATE_TEST_SUITE_P(ApplyEdit, ApplyEditTest, testing::ValuesIn(GetTestCaseList("applyEdit")));

    TEST_P(ApplyEditTest, ApplyEditCase)
    {
        TestParam param = GetParam();
        ASSERT_TRUE(LspApplyEditTest(param));
    }
}// namespace TestLspApplyEditTest