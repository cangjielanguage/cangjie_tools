// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Node/FeaturesDirectiveFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void FeaturesDirectiveFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto featuresDirective = As<ASTKind::FEATURES_DIRECTIVE>(node);
    AddFeaturesDirective(doc, *featuresDirective, level);
}

void FeaturesDirectiveFormatter::AddFeaturesDirective(
    Doc& doc, const Cangjie::AST::FeaturesDirective& featuresDirective, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!featuresDirective.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, featuresDirective.annotations, level, true);
    }

    doc.members.emplace_back(DocType::STRING, level, "features ");

    if (featuresDirective.featuresSet) {
        AddFeaturesSet(doc, *featuresDirective.featuresSet, level);
    }
}

void FeaturesDirectiveFormatter::AddFeaturesSet(Doc& doc, const Cangjie::AST::FeaturesSet& featuresSet, int level)
{
    doc.members.emplace_back(DocType::STRING, level, "{");

    auto& content = featuresSet.content;
    if (!content.empty()) {
        doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");

        for (size_t i = 0; i < content.size(); i++) {
            AddFeatureId(doc, content[i], level + 1);

            if (i < content.size() - 1) {
                doc.members.emplace_back(DocType::STRING, level + 1, ",");
                doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
            }
        }

        doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
    }

    doc.members.emplace_back(DocType::STRING, level, "}");
}

void FeaturesDirectiveFormatter::AddFeatureId(Doc& doc, const Cangjie::AST::FeatureId& featureId, int level)
{
    for (size_t i = 0; i < featureId.identifiers.size(); i++) {
        doc.members.emplace_back(DocType::STRING, level, featureId.identifiers[i].Val());
        if (i < featureId.identifiers.size() - 1) {
            doc.members.emplace_back(DocType::STRING, level, ".");
        }
    }
}
} // namespace Cangjie::Format
