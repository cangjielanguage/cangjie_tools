// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


#include "CangjieDemangle.h"
#ifndef BUILD_LIB_CANGJIE_DEMANGLE
#define BUILD_LIB_CANGJIE_DEMANGLE
#endif
#include "Demangler.h"
#include "StdString.h"

namespace Cangjie {
using DemangleMetaData = DemangleInfo<StdString>;

std::string DemangleData::GetPkgName() const { return pkgName; }

std::string DemangleData::GetFullName() const { return fullName; }

bool DemangleData::IsFunctionLike() const { return isFunctionLike; }

bool DemangleData::IsValid() const { return isValid; }

void DemangleData::SetPrivateDeclaration(bool isPrivate) { isPrivateDeclaration = isPrivate; }

bool DemangleData::IsPrivateDeclaration() const { return isPrivateDeclaration; }

DemangleData Demangle(const std::string& mangled, const std::string& scopeRes)
{
    auto demangler = Demangler<StdString>(mangled.c_str(), scopeRes);
    auto di = demangler.Demangle();
    auto dd = DemangleData{ di.GetPkgName(), di.GetFullName(demangler.ScopeResolution()), di.IsFunctionLike(),
        di.IsValid() };
    dd.SetPrivateDeclaration(di.isPrivateDeclaration);
    return dd;
}

DemangleData Demangle(const std::string& mangled) { return Demangle(mangled, "::"); }

DemangleData Demangle(const std::string& mangled, const std::string& scopeRes,
    const std::vector<std::string>& genericVec)
{
    auto demangler = Demangler<StdString>(mangled.c_str(), scopeRes);
    demangler.setGenericVec(genericVec);
    auto di = demangler.Demangle();
    auto dd = DemangleData{ di.GetPkgName(), di.GetFullName(demangler.ScopeResolution()), di.IsFunctionLike(),
        di.IsValid() };
    dd.SetPrivateDeclaration(di.isPrivateDeclaration);
    return dd;
}

DemangleData Demangle(const std::string& mangled, const std::vector<std::string>& genericVec)
{
    return Demangle(mangled, "::", genericVec);
}

DemangleData DemangleType(const std::string& mangled, const std::string& scopeRes)
{
    auto demangler = Demangler<StdString>(mangled, scopeRes);
    auto di = demangler.Demangle(true);
    auto dd = DemangleData{ di.GetPkgName(), di.GetFullName(demangler.ScopeResolution()), false,
        di.IsValid()};
    dd.SetPrivateDeclaration(di.isPrivateDeclaration);
    return dd;
}

DemangleData DemangleType(const std::string& mangled) { return DemangleType(mangled, "::"); }

#ifdef __OHOS__
char* CJ_MRT_Demangle(const char* functionName)
{
    DemangleData demangleData = Demangle(functionName);
    std::string pkgName = demangleData.GetPkgName();
    std::string demangledFunctionName =
        pkgName + std::string(pkgName.empty() ? "" : ".") + demangleData.GetFullName();
    int32_t len = strlen(demangledFunctionName.c_str());
    // demangleName should be freed by caller.
    char* demangleName = reinterpret_cast<char*>(malloc(len + 1));
    if (demangleName == nullptr) {
        return nullptr;
    }
    for (int i = 0; i < len; ++i) {
        demangleName[i] = demangledFunctionName[i];
    }
    demangleName[len] = '\0';
    return demangleName;
}
#endif
} // namespace Cangjie