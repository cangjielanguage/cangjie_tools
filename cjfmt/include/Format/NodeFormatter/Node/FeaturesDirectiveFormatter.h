// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_FEATURESDIRECTIVEFORMATTER_H
#define CJFMT_FEATURESDIRECTIVEFORMATTER_H

#include "Format/NodeFormatter/NodeFormatter.h"

namespace Cangjie::Format {
class FeaturesDirectiveFormatter : public NodeFormatter {
public:
    FeaturesDirectiveFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : NodeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddFeaturesDirective(Doc& doc, const Cangjie::AST::FeaturesDirective& featuresDirective, int level);
    void AddFeatureId(Doc& doc, const Cangjie::AST::FeatureId& featureId, int level);
    void AddFeaturesSet(Doc& doc, const Cangjie::AST::FeaturesSet& featuresSet, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_FEATURESDIRECTIVEFORMATTER_H
