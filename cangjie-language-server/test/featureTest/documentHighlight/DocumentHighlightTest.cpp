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

namespace TestLspDocumentHighlight {
    bool LspDocumentHighlightTest(TestParam param)
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
        std::vector<ark::DocumentHighlight> expect = ReadExpectedVector(param.baseFile);
        std::vector<ark::DocumentHighlight> actual = CreateDocumentHighlightStruct(ReadFileById(p->pathOut, param.id));
        std::string reason = "none";
        bool showErr = CheckDocumentHighlight(expect, actual, reason);
        if (!showErr) {
            nlohmann::json expLines = ReadExpectedResult(param.baseFile);
            nlohmann::json result = ReadFileById(p->pathOut, param.id);
            std::cout << "the false reason is : " << reason << std::endl;
            ShowDiff(expLines, result, param, p->messagePath);
        }
        return showErr;
    }

    class DocumentHighlightTest : public testing::TestWithParam<struct TestParam> {
    protected:
        void SetUp()
        {
            SetUpConfig("documentHighlight");
        }
    };

    INSTANTIATE_TEST_SUITE_P(DocumentHighlight, DocumentHighlightTest,
                             testing::ValuesIn(GetTestCaseList("documentHighlight")));

    TEST_P(DocumentHighlightTest, DocumentHighlightCase)
    {
        TestParam param = GetParam();
        ASSERT_TRUE(LspDocumentHighlightTest(param));
    }
}