// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_FILLTYPEPROCESSOR_H
#define CJFMT_FILLTYPEPROCESSOR_H

#include "Format/DocProcessor/DocProcessor.h"

namespace Cangjie::Format {
// FILL chooses a bounded, deterministic layout over syntax-aware soft breaks.
// Planning reads the Doc and suffix stack without changing either one.
class FillTypeProcessor : public DocProcessor {
public:
    static bool PlanFill(const Doc& root, int pos, const FormattingOptions& options,
        const std::vector<std::pair<Doc, Mode>>& suffixCommands, std::vector<Doc>& planned);
    explicit FillTypeProcessor(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DocProcessor(astToFormatSource, options) {}
    void DocToString(std::string& formatted, int& pos, std::pair<Doc, Mode>& current,
        std::vector<std::pair<Doc, Mode>>& leftCmd) override;
};
} // namespace Cangjie::Format

#endif // CJFMT_FILLTYPEPROCESSOR_H
