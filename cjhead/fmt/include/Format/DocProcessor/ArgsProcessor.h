// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_ARGSPROCESSOR_H
#define CJFMT_ARGSPROCESSOR_H
#include "Format/DocProcessor/DocProcessor.h"
#include <string>
#include <utility>
#include <vector>

namespace Cangjie::Format {
class ArgsProcessor : public DocProcessor {
public:
    explicit ArgsProcessor(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DocProcessor(astToFormatSource, options){};
    void DocToString(std::string& formatted, int& pos, std::pair<Doc, Mode>& current,
        std::vector<std::pair<Doc, Mode>>& leftCmd) override;
    void SoftLineProcessor(
        std::string& formatted, int& pos, std::pair<Doc, Mode>& current, size_t& i, bool& haveNotChangedLine);
    void SoftLineWithSpaceProcessor(
        std::string& formatted, int& pos, std::pair<Doc, Mode>& current, size_t& i, bool& haveNotChangedLine);
};
} // namespace Cangjie::Format
#endif // CJFMT_ARGSPROCESSOR_H
