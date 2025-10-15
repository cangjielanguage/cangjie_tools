// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef COMMON_DATA_H
#define COMMON_DATA_H

#include <string>
#include <vector>
#include "cangjie/CHIR/Type/CustomTypeDef.h"
#include "cangjie/CHIR/Type/Type.h"
namespace Cangjie::CodeCheck {
const std::string ANY_TYPE = "ANY_TYPE";
const std::string NOT_CARE = "NOT_CARE";

struct AstFuncInfo {
    std::string funcName;
    std::string parentTy;
    std::vector<std::string> params;
    std::string returnTy;
    std::string pkgName;
    AstFuncInfo(const std::string &name, const std::string &parent, const std::vector<std::string> &params,
        const std::string &returnTy, const std::string &pkgName)
        : funcName(name), parentTy(parent), params(params), returnTy(returnTy), pkgName(pkgName)
    {}
    bool operator == (const AstFuncInfo &rhs) const
    {
        return (this->funcName == rhs.funcName && this->parentTy == rhs.parentTy && this->params == rhs.params &&
            this->returnTy == rhs.returnTy && this->pkgName == rhs.pkgName);
    }
};

struct CHIRFuncInfo {
    std::string funcName;
    Ptr<CHIR::CustomType> parentTy;
    Ptr<CHIR::FuncType> funcTy;
    std::string pkgName;
    CHIRFuncInfo() = default;
    CHIRFuncInfo(const std::string &name, Ptr<CHIR::CustomType> parentTy, Ptr<CHIR::FuncType> funcTy,
        const std::string &pkgName)
        : funcName(name), parentTy(parentTy), funcTy(funcTy), pkgName(pkgName)
    {
    }
    bool operator==(const CHIRFuncInfo &rhs) const
    {
        return (this->funcName == rhs.funcName && this->parentTy == rhs.parentTy && this->funcTy == rhs.funcTy &&
            this->pkgName == rhs.pkgName);
    }
};

} // namespace Cangjie::CodeCheck

#endif