// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_SOFTLINETYPEPROCESSOR_H
#define CJFMT_SOFTLINETYPEPROCESSOR_H
#include "Format/DocProcessor/DocProcessor.h"

namespace Cangjie::Format {
class SoftLineTypeProcessor : public DocProcessor {
public:
    explicit SoftLineTypeProcessor(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DocProcessor(astToFormatSource, options){};
    void DocToString(std::string& formatted, int& pos, std::pair<Doc, Mode>& current,
        std::vector<std::pair<Doc, Mode>>& leftCmd) override;
};
} // namespace Cangjie::Format
#endif // CJFMT_SOFTLINETYPEPROCESSOR_H
