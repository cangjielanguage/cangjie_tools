// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/AnnotationFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
#include <string>
#include <set>

namespace Cangjie::Format {
using namespace Cangjie::AST;

void AnnotationFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto annotation = As<ASTKind::ANNOTATION>(node);
    AddAnnotation(doc, *annotation, level);
}

#ifdef CANGJIE_AD
namespace {
void AddIncludeOrExpectAnno(Doc& doc, const Cangjie::AST::Annotation& annotation, int level)
{
    doc.members.emplace_back(DocType::STRING, level, "[");
    if (!annotation.excepts.empty()) {
        std::set<std::string> excepts;
        for (auto& except : annotation.excepts) {
            (void)excepts.emplace(except->ref.GetIdentifierRawText());
        }
        Cangjie::Format::ASTToFormatSource::AddCustomization(doc, excepts, level);
    }
    if (!annotation.includes.empty()) {
        std::set<std::string> includes;
        for (auto& include : annotation.includes) {
            (void)includes.emplace(include->ref.GetIdentifierRawText());
        }
        Cangjie::Format::ASTToFormatSource::AddCustomization(doc, includes, level);
    }
    doc.members.emplace_back(DocType::STRING, level, "]");
}
} // namespace
#endif

void AnnotationFormatter::AddAnnotation(Doc& doc, const Cangjie::AST::Annotation& annotation, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    std::string compileTimeVisibleStr = annotation.isCompileTimeVisible ? "!" : "";
    doc.members.emplace_back(DocType::STRING, level, "@" + compileTimeVisibleStr + annotation.identifier);
    if (!annotation.args.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "[");
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        for (auto& arg : annotation.args) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(arg.get(), level + 1));
            if (arg->commaPos != INVALID_POSITION) {
                doc.members.emplace_back(DocType::STRING, level + 1, ",");
            }
            if (arg != annotation.args.back()) {
                doc.members.emplace_back(DocType::LINE, level + 1, " ");
            }
        }
        doc.members.emplace_back(DocType::LINE, level, "");
        doc.members.emplace_back(DocType::STRING, level, "]");
    }

#ifdef CANGJIE_AD
    if (std::to_string(annotation.stage) != "1" || !annotation.adAnnotation.empty()) {
        doc.members.emplace_back(DocType::STRING, level, " [");
    }

    if (std::to_string(annotation.stage) != "1") {
        doc.members.emplace_back(DocType::STRING, level, "stage: " + std::to_string(annotation.stage));
    }
    if (!annotation.adAnnotation.empty()) {
        if (std::to_string(annotation.stage) != "1") {
            doc.members.emplace_back(DocType::STRING, level, ", ");
        }
        doc.members.emplace_back(DocType::STRING, level, annotation.adAnnotation);
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }

    if (annotation.adAnnotation != "primal" && !annotation.adAnnotation.empty()) {
        AddIncludeOrExpectAnno(doc, annotation, level);
    }

    if (annotation.primal) {
        doc.members.emplace_back(DocType::STRING, level, annotation.primal->ref.identifier.GetRawText());
    }

    if (std::to_string(annotation.stage) != "1" || !annotation.adAnnotation.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "]");
    }
#endif

    if (annotation.condExpr) {
        doc.members.emplace_back(DocType::STRING, level, "[");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(annotation.condExpr.get(), level));
        doc.members.emplace_back(DocType::STRING, level, "]");
    }
}
} // namespace Cangjie::Format