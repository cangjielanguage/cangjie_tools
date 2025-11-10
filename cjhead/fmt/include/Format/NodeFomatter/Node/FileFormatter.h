// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_FILEFORMATTER_H
#define CJFMT_FILEFORMATTER_H
#include <unordered_map>
#include "Format/NodeFomatter/NodeFormatter.h"

namespace Cangjie::Format {
struct PositionHasher {
public:
    size_t operator()(const Cangjie::Position& pos) const { return static_cast<size_t>(pos.Hash64()); }
};

class FileFormatter : public NodeFormatter {
public:
    FileFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : NodeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void SetModifierOrAnnoToPosMap(const Cangjie::AST::File& file,
        std::unordered_map<Cangjie::Position, int, PositionHasher>& modifierOrAnnoToPosMap);
    void AddFile(Doc& doc, const Cangjie::AST::File& file, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_FILEFORMATTER_H
