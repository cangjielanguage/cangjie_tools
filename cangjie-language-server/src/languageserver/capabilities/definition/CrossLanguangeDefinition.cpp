// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CrossLanguangeDefinition.h"

bool ark::CrossLanguangeDefinition::getCrossSymbols(const CrossLanguageJumpParams &params, CrossSymbolsResult &result)
{
    const auto index = CompilerCangjieProject::GetInstance()->GetIndex();
    if (!index) {
        return false;
    }
    std::string outerName = params.outerName;
    index->FindCrossSymbolByName(params.packageName, params.name, params.isCombined,
        [outerName, &result](const lsp::CrossSymbol &crs) {
        if (crs.location.IsZeroLoc()) {
            return;
        }
        const auto realPath = crs.location.fileUri;
        if (EndsWith(realPath, ".macrocall")) {
            return;
        }
        if (!outerName.empty() && outerName != crs.containerName) {
            return;
        }
        Location loc{URI::URIFromAbsolutePath(realPath).ToString(),
                     TransformFromChar2IDE({crs.location.begin, crs.location.end})};
        result.locations.emplace(loc);
    });
    return true;
}