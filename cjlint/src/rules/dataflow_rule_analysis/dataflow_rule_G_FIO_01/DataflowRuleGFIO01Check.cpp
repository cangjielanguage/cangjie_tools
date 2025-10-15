// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DataflowRuleGFIO01Check.h"

using namespace Cangjie::CHIR;
using namespace Cangjie::CodeCheck;

template <> const std::optional<unsigned> Analysis<FIO01Domain>::blockLimit = std::nullopt;
template <> const AnalysisKind GenKillDomain<FIO01Domain>::mustOrMaybe = AnalysisKind::MAYBE;

static const std::vector<std::string> tempObject{"temp", "tmp", "temporary"};

static bool TempFileChecker(std::string path)
{
    (void)transform(path.begin(), path.end(), path.begin(), ::tolower);
    for (auto& item : tempObject) {
        if (path.find(item) != std::string::npos) {
            return true;
        }
    }
    return false;
}

static bool IsFileInit(const CHIRFuncInfo& info)
{
    return info.funcName == "init" && info.pkgName == "std.fs";
}

static bool IsFileDelete(const CHIRFuncInfo& info)
{
    return info.funcName == "remove" && (info.pkgName == "std.fs" || info.pkgName == "std.os.posix");
}

static bool IsFileCreate(const CHIRFuncInfo& info)
{
    return info.funcName == "creat" && info.pkgName == "std.posix";
}

static std::string GetFileName(const Value* value)
{
    if (!value->IsLocalVar()) {
        return "";
    }
    auto expr = Cangjie::StaticCast<Cangjie::CHIR::LocalVar*>(value)->GetExpr();
    if (expr->GetExprKind() == Cangjie::CHIR::ExprKind::CONSTANT) {
        auto constant = Cangjie::StaticCast<Cangjie::CHIR::Constant*>(expr);
        if (constant->IsStringLit()) {
            return constant->GetStringLitVal();
        }
    }
    return "";
}

static std::string GetFileName(const Apply* apply, unsigned index)
{
    return GetFileName(apply->GetArgs()[index]);
}

static std::string GetFileName(const ApplyWithException* apply, unsigned index)
{
    return GetFileName(apply->GetArgs()[index]);
}

template <typename T>
static void CheckApplyInAnalysisInit(T* apply, std::map<const std::string, size_t>& allocateIdxMap, size_t& allocateIdx)
{
    auto info = CommonFunc::GetCHIRFuncInfo(apply->GetCallee());
    if (!IsFileInit(info) && !IsFileCreate(info)) {
        return;
    }
    auto fileName = IsFileInit(info) ? GetFileName(apply, 1) : GetFileName(apply, 0);
    if (!fileName.empty() && TempFileChecker(fileName) && allocateIdxMap.find(fileName) == allocateIdxMap.end()) {
        allocateIdxMap.emplace(fileName, allocateIdx++);
    }
}

static void FIO01AnalysisHelper(
    Expression* expr, std::map<const std::string, size_t>& allocateIdxMap, size_t& allocateIdx)
{
    auto kind = expr->GetExprKind();
    if (kind == Cangjie::CHIR::ExprKind::APPLY) {
        auto apply = Cangjie::StaticCast<Cangjie::CHIR::Apply*>(expr);
        CheckApplyInAnalysisInit<Cangjie::CHIR::Apply>(apply, allocateIdxMap, allocateIdx);
    }
    if (kind == Cangjie::CHIR::ExprKind::APPLY_WITH_EXCEPTION) {
        auto apply = Cangjie::StaticCast<Cangjie::CHIR::ApplyWithException*>(expr);
        CheckApplyInAnalysisInit<Cangjie::CHIR::ApplyWithException>(apply, allocateIdxMap, allocateIdx);
    }
    if (expr->GetExprKind() == ExprKind::LAMBDA) {
        auto lambda = Cangjie::StaticCast<Cangjie::CHIR::Lambda*>(expr);
        auto blockGroup = lambda->GetBody();
        for (auto block : blockGroup->GetBlocks()) {
            for (auto expr : block->GetExpressions()) {
                FIO01AnalysisHelper(expr, allocateIdxMap, allocateIdx);
            }
        }
    }
}

FIO01Analysis::FIO01Analysis(Func* func, const std::vector<std::string>& files) : GenKillAnalysis(func)
{
    size_t allocateIdx = 0;
    for (auto fileName : files) {
        preFiles.emplace_back(fileName);
        allocateIdxMap.emplace(fileName, allocateIdx++);
    }
    for (auto bb : func->GetBody()->GetBlocks()) {
        for (auto expr : bb->GetExpressions()) {
            FIO01AnalysisHelper(expr, allocateIdxMap, allocateIdx);
        }
    }
    domainSize = allocateIdx;
}

template <typename T>
static void CheckApplyInGetFileName(
    T* apply, std::map<std::string, std::pair<Cangjie::Position, Cangjie::Position>>& fileNamePosMap)
{
    auto info = CommonFunc::GetCHIRFuncInfo(apply->GetCallee());
    if (!IsFileInit(info) && !IsFileCreate(info)) {
        return;
    }
    // 0 and 1 mean different index in the function's parameter list.
    size_t index = IsFileInit(info) ? 1 : 0;
    auto fileName = GetFileName(apply, index);
    auto pos = CommonFunc::GetCodePosition(apply->GetArgs()[index]);
    if (!fileName.empty() && TempFileChecker(fileName) && fileNamePosMap.find(fileName) == fileNamePosMap.end()) {
        fileNamePosMap.emplace(fileName, pos);
    }
}

static void GetFileNamePositionHelper(
    Expression* expr, std::map<std::string, std::pair<Cangjie::Position, Cangjie::Position>>& fileNamePosMap)
{
    auto kind = expr->GetExprKind();
    if (kind == Cangjie::CHIR::ExprKind::APPLY) {
        auto apply = Cangjie::StaticCast<Cangjie::CHIR::Apply*>(expr);
        CheckApplyInGetFileName<Cangjie::CHIR::Apply>(apply, fileNamePosMap);
    }
    if (kind == Cangjie::CHIR::ExprKind::APPLY_WITH_EXCEPTION) {
        auto apply = Cangjie::StaticCast<Cangjie::CHIR::ApplyWithException*>(expr);
        CheckApplyInGetFileName<Cangjie::CHIR::ApplyWithException>(apply, fileNamePosMap);
    }
    if (kind == ExprKind::LAMBDA) {
        auto lambda = Cangjie::StaticCast<Cangjie::CHIR::Lambda*>(expr);
        auto blockGroup = lambda->GetBody();
        for (auto block : blockGroup->GetBlocks()) {
            for (auto expr : block->GetExpressions()) {
                GetFileNamePositionHelper(expr, fileNamePosMap);
            }
        }
    }
}

static std::map<std::string, std::pair<Cangjie::Position, Cangjie::Position>> GetFileNamePosition(Func* func)
{
    std::map<std::string, std::pair<Cangjie::Position, Cangjie::Position>> fileNamePosMap;
    for (auto bb : func->GetBody()->GetBlocks()) {
        for (auto expr : bb->GetExpressions()) {
            GetFileNamePositionHelper(expr, fileNamePosMap);
        }
    }
    return fileNamePosMap;
}

void FIO01Analysis::InitializeFuncEntryState(FIO01Domain& state)
{
    state.kind = ReachableKind::REACHABLE;
    for (auto fileName : preFiles) {
        state.Gen(allocateIdxMap[fileName]);
    }
}

template <typename T>
static void CheckApplyInPropagate(
    const T* apply, FIO01Domain& state, std::map<const std::string, size_t>& allocateIdxMap)
{
    auto info = CommonFunc::GetCHIRFuncInfo(apply->GetCallee());
    if (IsFileDelete(info)) {
        auto fileName = GetFileName(apply, 0);
        if (!fileName.empty() && TempFileChecker(fileName) && allocateIdxMap.find(fileName) != allocateIdxMap.end()) {
            state.Kill(allocateIdxMap[fileName]);
        }
        return;
    }
    if (!IsFileInit(info) && !IsFileCreate(info)) {
        return;
    }
    auto fileName = IsFileInit(info) ? GetFileName(apply, 1) : GetFileName(apply, 0);
    if (!fileName.empty() && allocateIdxMap.find(fileName) != allocateIdxMap.end()) {
        state.Gen(allocateIdxMap[fileName]);
    }
}
void FIO01Analysis::PropagateExpressionEffect(FIO01Domain& state, const Expression* expression)
{
    auto kind = expression->GetExprKind();
    if (kind == ExprKind::APPLY) {
        auto apply = Cangjie::StaticCast<Cangjie::CHIR::Apply*>(expression);
        CheckApplyInPropagate<Cangjie::CHIR::Apply>(apply, state, allocateIdxMap);
    }
}

std::optional<Block*> FIO01Analysis::PropagateTerminatorEffect(FIO01Domain& state, const Terminator* terminator)
{
    auto kind = terminator->GetExprKind();
    if (kind == ExprKind::APPLY_WITH_EXCEPTION) {
        auto apply = Cangjie::StaticCast<Cangjie::CHIR::ApplyWithException*>(terminator);
        CheckApplyInPropagate<Cangjie::CHIR::ApplyWithException>(apply, state, allocateIdxMap);
    }
    return std::nullopt;
}

FIO01Domain FIO01Analysis::Bottom()
{
    return FIO01Domain(domainSize, &allocateIdxMap);
}

std::vector<std::string> DataflowRuleGFIO01Check::CheckDefaultParamFunc(CHIR::Func* func)
{
    std::vector<std::string> files{};
    auto analysis = std::make_unique<FIO01Analysis>(func);
    auto engine = Engine<FIO01Domain>(func, std::move(analysis));
    auto result = engine.IterateToFixpoint();
    if (!result) {
        return files;
    }
    const auto actionBeforeVisitExpr = [](const FIO01Domain&, Expression*, size_t) {};
    const auto actionAfterVisitExpr = [](const FIO01Domain&, Expression*, size_t) {};
    const auto actionOnTerminator = [&files](
                                        const FIO01Domain& state, const Terminator* terminator, std::optional<Block*>) {
        if (terminator->GetExprKind() == CHIR::ExprKind::EXIT) {
            for (auto iter : *state.GetMap()) {
                if (state.IsTrueAt(iter.second)) {
                    files.emplace_back(iter.first);
                }
            }
        }
    };
    result->VisitWith(actionBeforeVisitExpr, actionAfterVisitExpr, actionOnTerminator);
    return files;
}

void DataflowRuleGFIO01Check::CheckNormalFunc(CHIR::Func* func, const std::vector<std::string>& files,
    std::map<std::string, std::pair<Cangjie::Position, Cangjie::Position>>& posMap)
{
    auto analysis = std::make_unique<FIO01Analysis>(func, files);
    auto engine = Engine<FIO01Domain>(func, std::move(analysis));
    auto result = engine.IterateToFixpoint();
    if (!result) {
        return;
    }

    const auto actionBeforeVisitExpr = [](const FIO01Domain&, Expression*, size_t) {};
    const auto actionAfterVisitExpr = [](const FIO01Domain&, Expression*, size_t) {};
    const auto actionOnTerminator = [this, &posMap](
                                        const FIO01Domain& state, const Terminator* terminator, std::optional<Block*>) {
        if (terminator->GetExprKind() == CHIR::ExprKind::EXIT) {
            for (auto iter : *state.GetMap()) {
                if (state.IsTrueAt(iter.second)) {
                    auto returnPos = CommonFunc::GetCodePosition(terminator);
                    if (returnPos.first != Cangjie::Position(0, 0, 0)) {
                        diagEngine->Diagnose(posMap[iter.first].first, posMap[iter.first].second,
                            CodeCheckDiagKind::G_FIO_01, iter.first, returnPos.first.line, returnPos.first.column);
                    } else {
                        diagEngine->Diagnose(posMap[iter.first].first, posMap[iter.first].second,
                            CodeCheckDiagKind::G_FIO_01_compile_add_return, iter.first);
                    }
                }
            }
        }
    };
    result->VisitWith(actionBeforeVisitExpr, actionAfterVisitExpr, actionOnTerminator);
}

void DataflowRuleGFIO01Check::CheckBasedOnCHIR(CHIR::Package& package)
{
    // Collect the function translated from the default argument and bind it to the original function
    std::map<std::string, std::vector<std::string>> funcWithDefaultParams;
    std::map<std::string, std::map<std::string, std::pair<Cangjie::Position, Cangjie::Position>>> fileNamePosMap;
    for (auto func : package.GetGlobalFuncs()) {
        if (func->GetFuncKind() != FuncKind::DEFAULT_PARAMETER_FUNC || func->TestAttr(CHIR::Attribute::IMPORTED)) {
            continue;
        }
        auto files = CheckDefaultParamFunc(func);
        auto posMap = GetFileNamePosition(func);
        auto ownerFunc = func->GetParamDftValHostFunc()->GetIdentifier();
        if (funcWithDefaultParams.find(ownerFunc) == funcWithDefaultParams.end()) {
            funcWithDefaultParams[ownerFunc] = files;
            fileNamePosMap[ownerFunc] = posMap;
        } else {
            funcWithDefaultParams[ownerFunc].insert(funcWithDefaultParams[ownerFunc].end(), files.begin(), files.end());
            fileNamePosMap[ownerFunc].insert(posMap.begin(), posMap.end());
        }
    }

    for (auto func : package.GetGlobalFuncs()) {
        if (func->GetFuncKind() == FuncKind::DEFAULT_PARAMETER_FUNC || func->TestAttr(CHIR::Attribute::IMPORTED)) {
            continue;
        }
        // skip global var init func which is compilerAdd
        // Extract the first 5 characters to determine if it is a class automatically generated by the closure
        if (func->GetIdentifier().compare(0, 5, "@_CGV") == 0) {
            continue;
        }
        // Skip generic instantiation functions
        if (func->TestAttr(Attribute::GENERIC_INSTANTIATED)) {
            continue;
        }
        // Skip generic instantiation member functions
        if (func->GetOuterDeclaredOrExtendedDef() &&
            func->GetOuterDeclaredOrExtendedDef()->TestAttr(Attribute::GENERIC_INSTANTIATED)) {
            continue;
        }
        auto files = funcWithDefaultParams.find(func->GetIdentifier()) == funcWithDefaultParams.end()
            ? std::vector<std::string>()
            : funcWithDefaultParams[func->GetIdentifier()];
        auto posMap = GetFileNamePosition(func);
        if (fileNamePosMap.find(func->GetIdentifier()) != fileNamePosMap.end()) {
            fileNamePosMap[func->GetIdentifier()].insert(posMap.begin(), posMap.end());
            posMap = fileNamePosMap[func->GetIdentifier()];
        }
        CheckNormalFunc(func, files, posMap);
    }
}