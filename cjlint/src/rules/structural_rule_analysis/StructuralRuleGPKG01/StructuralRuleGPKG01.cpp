// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGPKG01.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGPKG01::FindImportNode(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)(
            [this](const ImportSpec &importSpec) {
                CheckImportItemName(importSpec);
            });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGPKG01::CheckImportItemName(const Cangjie::AST::ImportSpec& importSpec)
{
    if (importSpec.importPos == INVALID_POSITION || importSpec.TestAttr(AST::Attribute::COMPILER_ADD)) {
        return;
    }
    const auto& ic = importSpec.content;
    if (ic.kind == AST::ImportKind::IMPORT_ALL) {
        Diagnose(ic.begin, ic.end, CodeCheckDiagKind::G_PKG_01_avoid_wildcard,
            ic.prefixPaths.empty() ? std::string() : Utils::JoinStrings(ic.prefixPaths, "."));
    } else if (ic.kind == AST::ImportKind::IMPORT_MULTI) {
        for (const auto& item : ic.items) {
            if (item.kind == AST::ImportKind::IMPORT_ALL) {
                Diagnose(item.begin, item.end, CodeCheckDiagKind::G_PKG_01_avoid_wildcard,
                    item.prefixPaths.empty() ? std::string() : Utils::JoinStrings(item.prefixPaths, "."));
            }
        }
    }
}

void StructuralRuleGPKG01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindImportNode(node);
}
}
