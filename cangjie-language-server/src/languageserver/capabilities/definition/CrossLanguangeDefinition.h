// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CROSSLANGUANGEDEFINITION_H
#define CROSSLANGUANGEDEFINITION_H

#include "../../../json-rpc/Protocol.h"
#include "../../CompilerCangjieProject.h"
#include "cangjie/AST/Symbol.h"

namespace ark {
struct RegisterItem{
    Location definition;
    Location declaration;
    std::string registerName;
    int registerType;
};

struct ExportIDItem{
    std::string exportName;
    std::string containerName;
};

struct RegisterCrossSymbolsResult {
    std::list<RegisterItem> registerItems{};
};

struct CrossSymbolsResult {
    std::set<Location> locations{};
};
class CrossLanguangeDefinition {
public:
    static bool getCrossSymbols(const CrossLanguageJumpParams &params, CrossSymbolsResult &result);
    static bool GetExportSID(IDArray id, ExportIDItem &exportIdItem);
    static bool getRegisterCrossSymbols(const CrossLanguageJumpParams &params, RegisterCrossSymbolsResult &result);
};
}

#endif // CROSSLANGUANGEDEFINITION_H

