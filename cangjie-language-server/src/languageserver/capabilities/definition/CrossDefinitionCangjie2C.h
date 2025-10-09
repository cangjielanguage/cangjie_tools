// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_CROSSDEFINITIONCANGJIE2C_H
#define LSPSERVER_CROSSDEFINITIONCANGJIE2C_H

#include <string>
#include <vector>
#include <unordered_map>
#include "cangjie/AST/Types.h"
#include "cangjie/AST/Node.h"

namespace ark {

// extension crossLanguageJump
struct message {
    std::string targetLanguage;
    std::string functionName;
    std::vector<std::string> functionParameters{};
    std::string retType;
};

class CrossDefinitionCangjie2C {
public:
    static void Cangjie2CGetFuncMessage(std::vector<message> &CrossMessage, Ptr<Cangjie::AST::FuncDecl> funcDecl);

    static std::string GetCType(const Cangjie::AST::Ty *ty, bool isSimple = false);

    static std::string TypeVarray(const Cangjie::AST::Ty *ty, bool isSimple);

    static std::string TypeCpointer(const Cangjie::AST::Ty *ty, bool isSimple);
};
}

#endif // LSPSERVER_CROSSDEFINITIONCANGJIE2C_H
