// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Expr/BlockFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void BlockFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto block = As<ASTKind::BLOCK>(node);
    AddBlock(doc, *block, level, funcOptions);
}

void BlockFormatter::AddBlock(Doc& doc, const Cangjie::AST::Block& block, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (funcOptions.isLambda) {
        AddBlockIsLambda(doc, block, level);
        return;
    }
    if (block.leftCurlPos == INVALID_POSITION && block.rightCurlPos == INVALID_POSITION) {
        // match case block
        astToFormatSource.AddBodyMembers(doc, block.body, level);
        return;
    }

    if (block.body.empty()) {
        astToFormatSource.AddEmptyBody(doc, block, level, block.leftCurlPos.line == block.rightCurlPos.line);
        return;
    }

    if (block.TestAttr(Attribute::UNSAFE)) {
        doc.members.emplace_back(DocType::STRING, level, "unsafe");
        block.body.size() == 1 &&
            block.body.back()->end.line == block.rightCurlPos.line ?
            AddSameLineCurl(doc, block, level) : AddDiffLineCurl(doc, block, level);
        return;
    }
    AddDiffLineCurl(doc, block, level);
}

void BlockFormatter::AddBlockIsLambda(Doc& doc, const Cangjie::AST::Block& block, int level)
{
    int lastEndLine = -1;
    for (auto& n : block.body) {
        if (lastEndLine != -1) {
            if (n->begin.line > lastEndLine + 1) {
                doc.members.emplace_back(DocType::SEPARATE, level, "");
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level));

        lastEndLine = n->end.line;
    }
}

void BlockFormatter::AddSameLineCurl(Doc& doc, const Cangjie::AST::Block& block, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " { ");
    for (auto& n : block.body) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, " }");
    if (block.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

void BlockFormatter::AddDiffLineCurl(Doc& doc, const Cangjie::AST::Block& block, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " {");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    astToFormatSource.AddBodyMembers(doc, block.body, level + 1);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
    if (block.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format