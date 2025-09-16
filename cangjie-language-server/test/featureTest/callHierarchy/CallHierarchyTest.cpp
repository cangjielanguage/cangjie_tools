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

namespace TestCallHierarchy {

    bool LspCallHierarchyTest(TestParam param){
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
        CallHierarchyResult expectPrepare;
        ReadExpectedCallHierarchyResult(param.baseFile, expectPrepare);
        CallHierarchyResult actualPrepare;
        CreatePrePareCallHierarchyStruct(ReadFileById(p->pathOut, param.preId),actualPrepare);

        if (CreateMsg(p->pathIn, testFile, rootUri, isMultiModule,std::to_string(actualPrepare.symbolId)) != true) {
            return false;
        }
        StartLspServer(SingleInstance::GetInstance()->useDB);

        CallHierarchyResult expect;
        ReadExpectedCallHierarchyResult(param.baseFile,expect);
        CallHierarchyResult actual;
        CreateCallHierarchyStruct(ReadFileById(p->pathOut, param.id),actual);

        /* if case is diff show info */
        std::string reason;
        bool showErr = CheckCallHierarchyResult(expect, actual, reason);
        if (!showErr) {
            nlohmann::json expLines = ReadExpectedResult(param.baseFile);
            nlohmann::json result = ReadFileById(p->pathOut, param.id);
            std::cout << "the false reason is : " << reason << std::endl;
            ShowDiff(expLines, result, param, p->messagePath);
        }
        return showErr;
    }

    class CallHierarchyTest : public testing::TestWithParam<struct TestParam> {
    protected:
        void SetUp()
        {
            SetUpConfig("callHierarchy");
        }
    };

    INSTANTIATE_TEST_SUITE_P(CallHierarchy, CallHierarchyTest, testing::ValuesIn(GetTestCaseList("callHierarchy")));

    TEST_P(CallHierarchyTest, CallHierarchyCase)
    {
        TestParam param = GetParam();
        ASSERT_TRUE(LspCallHierarchyTest(param));
    }

}
