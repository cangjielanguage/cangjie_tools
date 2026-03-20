// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Node/AnnotationFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void AnnotationFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto annotation = As<ASTKind::ANNOTATION>(node);
    AddAnnotation(doc, *annotation, level);
}

void AnnotationFormatter::AddAnnotation(Doc& doc, const Cangjie::AST::Annotation& annotation, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    std::string compileTimeVisibleStr = annotation.isCompileTimeVisible ? "!" : "";
    doc.members.emplace_back(DocType::STRING, level, "@" + compileTimeVisibleStr + annotation.identifier);
    if (!annotation.args.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "[");
        for (auto& arg : annotation.args) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(arg.get(), level));
            if (arg->commaPos != INVALID_POSITION) {
                doc.members.emplace_back(DocType::STRING, level, ",");
            }
            if (arg != annotation.args.back()) {
                doc.members.emplace_back(DocType::STRING, level, " ");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, "]");
    }
    if (!annotation.attrs.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "[");
        auto& last_attr = annotation.attrs.back();
        for (auto& attr : annotation.attrs) {
            doc.members.emplace_back(DocType::STRING, level, attr.Value());
            if (&attr != &last_attr) {
                doc.members.emplace_back(DocType::STRING, level, ", ");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, "]");
    }
    if (annotation.condExpr) {
        doc.members.emplace_back(DocType::STRING, level, "[");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(annotation.condExpr.get(), level));
        doc.members.emplace_back(DocType::STRING, level, "]");
    }
}
} // namespace Cangjie::Format