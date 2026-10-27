// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_DOCPROCESSOR_H
#define CJFMT_DOCPROCESSOR_H

#include "Format/Doc.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/AST/Node.h"
#include "cangjie/Basic/Display.h"
#include "cangjie/Utils/Utils.h"

namespace Cangjie::Format {
class ASTToFormatSource;

class DocProcessor {
public:
    explicit DocProcessor(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : options(options), astToFormatSource(astToFormatSource){};
    virtual ~DocProcessor() = default;
    virtual void DocToString(std::string& formatted, int& pos, std::pair<Doc, Mode>& current,
        std::vector<std::pair<Doc, Mode>>& leftCmd) = 0;
    static int AdvanceColumn(const std::string& text, int column);
    static int MeasureFlat(const Doc& doc);
    static int DefaultBreakPenalty(BreakKind kind);

protected:
    static void RebaseToOutputLine(Doc& doc, const std::string& formatted, int indentWidth);
    bool Fits(const std::pair<Doc, Mode>& next, int rem);
    bool Fits(const std::vector<std::pair<Doc, Mode>>& commands, int rem);
    int ConsumeFitCommand(const std::pair<const Doc*, Mode>& current, int& rem,
        std::vector<std::pair<const Doc*, Mode>>& fitCmd);
    int CalculateDocLen(Doc& doc);
    // Returns the flat display width when a document can be emitted without a hard line.
    // A negative result means that the candidate contains a forced line break.
    static int MeasureInlineSuffix(const std::vector<std::pair<Doc, Mode>>& commands);

    FormattingOptions& options;
    ASTToFormatSource& astToFormatSource;
};
} // namespace Cangjie::Format
#endif // CJFMT_DOCPROCESSOR_H
