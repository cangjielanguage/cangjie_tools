// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_COMMONFUNC_H
#define CANGJIECODECHECK_COMMONFUNC_H

#include <string>
#include <vector>
#include "cangjie/AST/Node.h"
#include "cangjie/CHIR/Expression/Terminator.h"
#include "cangjie/CHIR/Package.h"
#include "cangjie/CHIR/Type/ClassDef.h"
#include "cangjie/CHIR/Type/EnumDef.h"
#include "cangjie/CHIR/Type/StructDef.h"
#include "cangjie/CHIR/Type/Type.h"
#include "common/CJLintCompilerInvocation.h"
#include "common/CommonData.h"
#include "common/ConfigContext.h"
#include "common/DiagnosticEngine.h"
#include "nlohmann/json.hpp"

namespace Cangjie::CodeCheck {
const int OK = 0;
const int ERR = 255;
const int JSON_READ_ERR = 254;

class CommonFunc {
public:
    CommonFunc() = default;
    ~CommonFunc() = default;
    using PositionPair = std::pair<Cangjie::Position, Cangjie::Position>;
    static PositionPair GetCodePosition(const CHIR::Expression* expr);
    static PositionPair GetCodePosition(const CHIR::Value* value);
    static PositionPair GetCodePosition(const CHIR::DebugLocation loc);
    static void getAbsPath(const std::string& path, std::optional<std::string>& absPath);
    static int ReadJsonFileToJsonInfo(
        const std::string& jsonPath, ConfigContext& configContext, nlohmann::json& jsonInfo);
    static bool FindCHIRFunction(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool FindCHIRFunction(const CHIR::Value* value, const AstFuncInfo& funcInfo);
    static std::string GetChainMemberPathName(const CHIR::GetElementRef* getElementRef);
    static std::string GetChainMemberName(const CHIR::GetElementRef* getElementRef);
    static std::string GetFuncDeclParentTyName(Ptr<const Cangjie::AST::FuncDecl> pFuncDecl);
    static bool HasEnding(std::string const& fullString, std::string const& ending);
    static std::string PrintType(const CHIR::Type* ty);
    static std::string PrintTypeWithArgs(const CHIR::Type* ty);
    static CHIRFuncInfo GetCHIRFuncInfo(const CHIR::Value* value);
    static std::tuple<std::string, CHIR::CustomType*, CHIR::FuncType*> GetCHIRFuncInfoKindFunc(
        const CHIR::Value* value);
    static std::tuple<std::string, CHIR::CustomType*, CHIR::FuncType*> GetCHIRFuncInfoKindImpFunc(
        const CHIR::Value* value);
    static bool CheckFuncName(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool CheckArgumentsTy(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo, bool inClass = false);
    static bool CheckReturnTy(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool CheckParentTy(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool CheckPkgName(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool IsGenericInstantated(const CHIR::Func* func);
    static bool IsStdDerivedMacro(CodeCheckDiagnosticEngine* engine, const Cangjie::Position& pos);
private:
    static std::vector<uint64_t> emptyPath;
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_COMMONFUNC_H
