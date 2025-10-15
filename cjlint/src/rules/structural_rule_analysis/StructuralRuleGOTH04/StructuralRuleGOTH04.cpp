// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGOTH04.h"
#include "cangjie/Basic/Match.h"
#include "common/CommonFunc.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;
using namespace std;

constexpr const char* JSONPATH = "/config/structural_rule_G_OTH_04.json";

bool StructuralRuleGOTH04::SenInfoFilter(const std::string& key, const std::string& text)
{
    if (!jsonInfo.contains(key)) {
        Errorln(JSONPATH, " read json data failed!");
        return false;
    }

    auto lowercaseText = text;
    (void)transform(lowercaseText.begin(), lowercaseText.end(), lowercaseText.begin(), ::tolower);
    for (const auto& item : jsonInfo[key]) {
        if (lowercaseText.find(item.get<std::string>()) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void StructuralRuleGOTH04::FindVarDecl(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const VarDecl& varDecl) { AnalyzeVarDecl(varDecl); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}
void StructuralRuleGOTH04::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    FindVarDecl(node);
}
bool StructuralRuleGOTH04::IsSensitiveDataVar(const string& text)
{
    return SenInfoFilter("SensitiveDataVar", text);
}

void StructuralRuleGOTH04::VarDeclTypeAnalysis(Ptr<Ty> ty, bool& flag)
{
    switch (ty->kind) {
        case AST::TypeKind::TYPE_STRUCT: {
            TypeStructAnalysis(ty, flag);
            break;
        }
        case AST::TypeKind::TYPE_ENUM: {
            TypeEnumAnalysis(ty, flag);
            break;
        }
        case AST::TypeKind::TYPE_TUPLE: {
            TypeTupleAnalysis(ty, flag);
            break;
        }
        case AST::TypeKind::TYPE_CLASS: {
            TypeClassAnalysis(ty, flag);
            break;
        }
        default: {
            break;
        }
    }
}
void StructuralRuleGOTH04::AnalyzeVarDecl(const VarDecl& varDecl)
{
    if (!IsSensitiveDataVar(varDecl.identifier)) {
        return;
    }
    if (!varDecl.ty) {
        return;
    }

    bool flag = false;
    VarDeclTypeAnalysis(varDecl.ty, flag);
    if (flag) {
        Diagnose(varDecl.identifier.Begin(), varDecl.identifier.End(),
            CodeCheckDiagKind::G_OTH_04_avoid_use_string_type_to_store_sensitive_data, varDecl.identifier.Val());
    }
}
void StructuralRuleGOTH04::TypeStructAnalysis(Ptr<Ty> ty, bool& flag)
{
    if (ty->IsString()) {
        flag = true;
        return;
    }
    if (!ty->IsStructArray()) {
        return;
    }
    IsIncludeStringType(ty, flag);
}
void StructuralRuleGOTH04::TypeEnumAnalysis(Ptr<Cangjie::AST::Ty> ty, bool& flag)
{
    if (!ty->IsCoreOptionType()) {
        return;
    }
    IsIncludeStringType(ty, flag);
}
void StructuralRuleGOTH04::TypeTupleAnalysis(Ptr<Cangjie::AST::Ty> ty, bool& flag)
{
    if (!ty->IsTuple()) {
        return;
    }
    IsIncludeStringType(ty, flag);
}
void StructuralRuleGOTH04::TypeClassAnalysis(Ptr<Ty> ty, bool& flag)
{
    IsIncludeStringType(ty, flag);
}

void StructuralRuleGOTH04::IsIncludeStringType(Ptr<Ty> ty, bool& flag)
{
    for (auto& item : ty->typeArgs) {
        if (item->IsString()) {
            flag = true;
            break;
        }
        VarDeclTypeAnalysis(item, flag);
    }
}
} // namespace Cangjie::CodeCheck