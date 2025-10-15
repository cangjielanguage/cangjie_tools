// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGFMT13.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGFMT13::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindComments(node);
}
void StructuralRuleGFMT13::FindComments(Ptr<Cangjie::AST::Node>& node)
{
    if (!node) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::FILE) {
            auto pFile = As<ASTKind::FILE>(node.get());
            GetTopLevelComments(pFile);
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

bool StructuralRuleGFMT13::ContainsCopyright(const std::string& str)
{
    std::regex pattern(R"(Copyright\s*\(c\)|版权所有\s*\(c\))");

    return std::regex_search(str, pattern);
}

void StructuralRuleGFMT13::AnalyzeComments(std::vector<AST::CommentGroup>& leadingComments, Cangjie::Position& pos)
{
    if (leadingComments.empty()) {
        Diagnose(pos, pos, CodeCheckDiagKind::G_FMT_13_header_comments_copyright_01);
        return;
    }
    auto comment = leadingComments[0].cms[0].info.Value();
    if (!ContainsCopyright(comment)) {
        Diagnose(pos, pos, CodeCheckDiagKind::G_FMT_13_header_comments_copyright_01);
    } else {
        bool isBlockComment = leadingComments[0].cms[0].kind == CommentKind::BLOCK;
        if (!isBlockComment) {
            Diagnose(pos, pos, CodeCheckDiagKind::G_FMT_13_header_comments_copyright_02);
        }
    }
}

void StructuralRuleGFMT13::GetTopLevelComments(Ptr<Cangjie::AST::File> pFile)
{
    if (pFile->package) {
        AnalyzeComments(pFile->package->comments.leadingComments, pFile->begin);
        return;
    }
    if (!pFile->imports.empty() && !pFile->imports[0]->begin.IsZero()) {
        AnalyzeComments(pFile->imports[0]->comments.leadingComments, pFile->begin);
        return;
    }
    if (!pFile->decls.empty()) {
        Ptr<Decl> decl = pFile->decls[0]->GetDesugarDecl() ? pFile->decls[0]->GetDesugarDecl() : pFile->decls[0].get();
        if (!decl) {
            return;
        }
        AnalyzeComments(decl->comments.leadingComments, pFile->begin);
        return;
    }
}
} // namespace Cangjie::CodeCheck