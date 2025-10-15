// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_REGEXRULE_H
#define CANGJIECODECHECK_REGEXRULE_H
#include <fstream>
#include <iostream>
#include <regex>
#include "common/ConfigContext.h"
#include "StructuralRule.h"

namespace Cangjie::CodeCheck {
enum class HardcodeType {
    IP = 0,
    URL = 1,
    EMAIL = 2
};

struct ResultInfo {
    int type = -1;
    int line = -1;
    int column = -1;
    int endLine = -1;
    int endColumn = -1;
    std::string result;
    std::string filepath;
};

struct RegexInfo {
    std::map<int, std::regex> regex;
    std::string filepath = "";
};

class RegexRule : public StructuralRule {
public:
    std::vector<ResultInfo> resultVector;
    explicit RegexRule(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~RegexRule() override = default;
    std::vector<ResultInfo> InitRegexInfo(Ptr<Cangjie::AST::Node> node, const std::map<int, std::regex> reg);

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override = 0;

private:
    ResultInfo resultInfo;
    RegexInfo regexInfo;
    void GetFileFromNode(Ptr<Cangjie::AST::Node> node);
    void GetMatchedPatternFromFile();
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_REGEXRULE_H
