// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DATAFLOW_RULE_G_OTH_01_CHECK_H
#define DATAFLOW_RULE_G_OTH_01_CHECK_H

#include "../DataflowRule.h"
#include "common/CommonFunc.h"
#include "common/ConfigContext.h"
#include "common/DiagnosticEngine.h"
#include "nlohmann/json.hpp"

namespace Cangjie::CodeCheck {
class DataflowRuleGOTH01Check : public DataflowRule {
public:
    DataflowRuleGOTH01Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleGOTH01Check(CodeCheckDiagnosticEngine* diagEngine);
    ~DataflowRuleGOTH01Check() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package& package) override;

private:
    nlohmann::json sensitiveKeys;
    template <typename T> void CheckApplyOrInvoke(Ptr<T> apply, const CHIR::ConstDomain& state);
    static std::set<std::string> logMsgGetter;
};
} // namespace Cangjie::CodeCheck
#endif