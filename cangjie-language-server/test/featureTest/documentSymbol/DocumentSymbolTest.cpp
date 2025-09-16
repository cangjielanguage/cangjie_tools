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

namespace TestLspDocumentSymbol {
    bool LspDocumentSymbolTest(TestParam param)
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
        nlohmann::json expect = ReadExpectedResult(param.baseFile);
        nlohmann::json actual = ReadFileById(p->pathOut, param.id);
        std::string reason = "none";
        bool showErr = CheckDocumentSymbolResult(expect, actual, reason);
        if (!showErr) {
            std::cout << "the false reason is : " << reason << std::endl;
            ShowDiff(expect, actual, param, p->messagePath);
        }
        return showErr;
    }

    class DocumentSymbolTest : public testing::TestWithParam<struct TestParam> {
    protected:
        void SetUp()
        {
            SetUpConfig("documentSymbol");
        }
    };

    INSTANTIATE_TEST_SUITE_P(DocumentSymbol, DocumentSymbolTest,
            testing::ValuesIn(GetTestCaseList("documentSymbol")));

    TEST_P(DocumentSymbolTest, DocumentSymbolCase)
    {
        TestParam param = GetParam();
        ASSERT_TRUE(LspDocumentSymbolTest(param));
    }
}