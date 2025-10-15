// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGSER.h"
#include "cangjie/Basic/Match.h"

using namespace Cangjie::CodeCheck;
using namespace Cangjie::Meta;
using namespace Cangjie::AST;

void StructuralRuleGSER::FindExtendSer(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker1(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const ExtendDecl &extendDecl) {
                for (auto &it : extendDecl.inheritedTypes) {
                    if ((it->ToString()).find("Serializable<") != std::string::npos) {
                        (void)extendSers.insert(extendDecl.extendedType->ToString());
                        return VisitAction::SKIP_CHILDREN;
                    }
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker1.Walk();
}
