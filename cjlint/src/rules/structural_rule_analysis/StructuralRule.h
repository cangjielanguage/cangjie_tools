// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_H

#include <functional>
#include <vector>

#include "cangjie/AST/ASTContext.h"
#include "cangjie/AST/Node.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/DiagnosticEngine.h"
#include "common/CJLintCompilerInstance.h"
#include "cangjie/Utils/Utils.h"
#include "common/ConfigContext.h"
#include "common/DiagnosticEngine.h"
#include "rules/Rule.h"

namespace Cangjie::CodeCheck {
class StructuralRule : public Rule {
public:
    explicit StructuralRule(CodeCheckDiagnosticEngine *diagEngine) : Rule(diagEngine) {};
    void DoAnalysis(CJLintCompilerInstance *instance) override;

protected:
    virtual void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) = 0;
    template <typename... Args> void Diagnose(const Position pos, CodeCheckDiagKind kind, const Args... args)
    {
        diagEngine->Diagnose(pos, kind, args...);
    }
    template <typename... Args> void Diagnose(const Position start, const Position end, CodeCheckDiagKind kind,
        const Args... args)
    {
        diagEngine->Diagnose(start, end, kind, args...);
    }
    template <typename... Args>
    void Diagnose(const std::string filePath, int line, int column, CodeCheckDiagKind kind, const Args... args)
    {
        Cangjie::Position pos;
        pos.fileID = static_cast<unsigned int>(diagEngine->GetSourceManager().GetFileID(filePath));
        pos.line = line;
        pos.column = column;
        diagEngine->Diagnose(pos, kind, args...);
    }
    template <typename... Args>
    void Diagnose(const std::string filePath, int line, int column, int endLine, int endColumn, CodeCheckDiagKind kind,
        const Args... args)
    {
        Cangjie::Position start, end;
        start.fileID = static_cast<unsigned int>(diagEngine->GetSourceManager().GetFileID(filePath));
        start.line = line;
        start.column = column;
        end.fileID = static_cast<unsigned int>(diagEngine->GetSourceManager().GetFileID(filePath));
        end.line = endLine;
        end.column = endColumn;
        diagEngine->Diagnose(start, end, kind, args...);
    }
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_H
