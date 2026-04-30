// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_EXTRACTINTERFACE_H
#define CANGJIE_LSP_EXTRACTINTERFACE_H

#include "../../../../json-rpc/Protocol.h"
#include "../Tweak.h"

namespace ark {

class ExtractInterface : public Tweak {
public:
    enum class ExtractInterfaceError {
        INVALID_TARGET = 2,
        EMPTY_METHODS = 3
    };

    const std::string Id() const override
    {
        return "ExtractInterface";
    };

    std::string Title() const override
    {
        return "Extract to interface";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Tweak::Selection &sel) override;

    std::optional<Tweak::Effect> Apply(const Tweak::Selection &sel) override;

private:
};
} // namespace ark

#endif // CANGJIE_LSP_EXTRACTINTERFACE_H
