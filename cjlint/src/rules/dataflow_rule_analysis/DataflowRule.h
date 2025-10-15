// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DATAFLOW_RULE_ANALYSIS_DATAFLOW_RULE_H
#define DATAFLOW_RULE_ANALYSIS_DATAFLOW_RULE_H

#include "common/CJLintCompilerInstance.h"
#include "common/ConfigContext.h"
#include "common/DiagnosticEngine.h"
#include "rules/Rule.h"


#include "cangjie/CHIR/Analysis/AnalysisWrapper.h"
#include "cangjie/CHIR/Analysis/ConstAnalysis.h"

namespace Cangjie::CodeCheck {
class DataflowRule : public Rule {
public:
    explicit DataflowRule(CodeCheckDiagnosticEngine* diagEngine) : Rule(diagEngine){};
    ~DataflowRule() override = default;
    void DoAnalysis(CJLintCompilerInstance* instance) override;

protected:
    using AnalysisWrapper = CHIR::AnalysisWrapper<CHIR::ConstAnalysis, CHIR::ConstDomain>;
    AnalysisWrapper* analysisWrapper{nullptr};
    virtual void CheckBasedOnCHIR(CHIR::Package& package) = 0;
};
} // namespace Cangjie::CodeCheck

#endif // DATAFLOW_RULE_ANALYSIS_DATAFLOW_RULE_H
