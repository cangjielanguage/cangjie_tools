// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CJHeadCompilerInstance.h"

using namespace Cangjie;
using namespace Cangjie::AST;

namespace CJHead {
auto CJHeadCompilerInstance::Compile(CompileStage stage) -> bool
{
    return CompilerInstance::Compile(stage);
}
}  // namespace CJHead
