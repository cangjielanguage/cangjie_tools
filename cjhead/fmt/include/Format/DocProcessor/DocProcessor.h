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
#include <string>
#include <utility>
#include <vector>

namespace Cangjie::Format {
class ASTToFormatSource;

class DocProcessor {
public:
    explicit DocProcessor(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : astToFormatSource(astToFormatSource), options(options){};
    virtual ~DocProcessor() = default;
    virtual void DocToString(std::string& formatted, int& pos, std::pair<Doc, Mode>& current,
        std::vector<std::pair<Doc, Mode>>& leftCmd) = 0;

protected:
    bool Fits(const std::pair<Doc, Mode>& next, int rem);
    int CalculateDocLen(Doc& doc);

    FormattingOptions& options;
    ASTToFormatSource& astToFormatSource;
};
} // namespace Cangjie::Format
#endif // CJFMT_DOCPROCESSOR_H
