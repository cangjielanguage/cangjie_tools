// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_CALLBACK_H
#define LSPSERVER_CALLBACK_H
#include <iostream>
#include <vector>
#include "../../json-rpc/Protocol.h"
#include "../logger/Logger.h"
#include "cangjie/Basic/DiagnosticEngine.h"

namespace Cangjie {
struct TextEdit {
    Position start;
    Position end;
    std::string newText;
    friend bool operator!=(const TextEdit& teOne, const TextEdit& teTwo)
    {
        return !(teOne.start == teTwo.start && teOne.end == teTwo.end && teOne.newText == teTwo.newText);
    }
    friend bool operator==(const TextEdit& teOne, const TextEdit& teTwo)
    {
        return teOne.start == teTwo.start && teOne.end == teTwo.end && teOne.newText == teTwo.newText;
    }
};
} // namespace Cangjie

namespace ark {
class Callbacks {
public:
    virtual ~Callbacks() = default;

    virtual std::vector<DiagnosticToken> GetDiagsOfCurFile(std::string) = 0;
    virtual void UpdateDiagnostic(std::string, DiagnosticToken) = 0;
    virtual void RemoveDiagOfCurPkg(const std::string &dirName) = 0;
    virtual void ReadyForDiagnostics(std::string, int64_t, std::vector <DiagnosticToken>) = 0;
    virtual std::string GetContentsByFile(const std::string &file) = 0;
    virtual int64_t GetVersionByFile(const std::string &file) = 0;
    virtual void RemoveDocByFile(const std::string &file)  = 0;
    virtual bool NeedReParser(const std::string &file) = 0;
    virtual void UpdateDocNeedReparse(const std::string &file, int64_t version, bool needReParser) = 0;
    virtual void AddDocWhenInitCompile(const std::string &file) = 0;
    virtual void ReportCjoVersionErr(std::string message) = 0;
    virtual void PublishCompletionTip(const CompletionTip &params) = 0;
    bool isRenameDefined = false;
    std::string path = "";
};
} // namespace ark
#endif // LSPSERVER_CALLBACK_H
