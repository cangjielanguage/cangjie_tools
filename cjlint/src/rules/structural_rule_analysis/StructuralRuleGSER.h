// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURAL_RULE_G_SER_H
#define CANGJIECODECHECK_STRUCTURAL_RULE_G_SER_H

#include "StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGSER : public StructuralRule {
public:
    explicit StructuralRuleGSER(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGSER() override = default;

protected:
    std::set<std::string> extendSers;
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override = 0;
    void FindExtendSer(Ptr<Cangjie::AST::Node> node);
    /*
     * Check whether the type implements the serial number interface.
     */
    template <typename T> bool IsImpSerializable(const T &decl, std::set<Ptr<AST::Decl>> declSet = {}) const
    {
        if (extendSers.find(decl.identifier) != extendSers.end()) {
            return true;
        }
        for (auto &it : decl.inheritedTypes) {
            if ((it->ToString()).find("Serializable<") != std::string::npos) {
                return true;
            }
            if (it->ty && it->ty->kind == AST::TypeKind::TYPE_CLASS) {
                auto classDecl = StaticCast<AST::ClassTy>(it->ty)->decl;
                if (declSet.count(classDecl) > 0) {
                    continue;
                }
                declSet.insert(classDecl);
                if (IsImpSerializable(*classDecl, declSet)) {
                    return true;
                }
            }
            if (it->ty && it->ty->kind == AST::TypeKind::TYPE_INTERFACE) {
                auto interfaceDecl = StaticCast<AST::InterfaceTy>(it->ty)->decl;
                if (declSet.count(interfaceDecl) > 0) {
                    continue;
                }
                declSet.insert(interfaceDecl);
                if (IsImpSerializable(*interfaceDecl, declSet)) {
                    return true;
                }
            }
        }
        return false;
    }
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURAL_RULE_G_SER_H
