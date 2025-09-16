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
    bool showErr = CheckApplyEditResult(expLines, result, info);
    if (!showErr) {
        std::cout << "the false reason is : " << info << std::endl;
        ShowDiff(expLines, result, param, p->messagePath);
    }
    return showErr;
}
} // namespace TestLspApplyEditTest