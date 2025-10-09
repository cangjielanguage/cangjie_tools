// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CodeLensImpl.h"

namespace ark {
std::string CodeLensImpl::curFilePath;
const std::string TITLE_RUN = "run";
const std::string TITLE_DEBUG = "debug";
const std::string COMMAND_TEST_RUN = "cangjie.test.run";
const std::string COMMAND_TEST_DEBUG = "cangjie.test.debug";
const std::vector<std::string> CODE_LENS_TARGET = {"Test", "TestCase"};

void HandleTestResult(std::vector<CodeLens> &result, std::string title, std::string command,
                      ExecutableRange executableRange)
{
    std::set<ExecutableRange> arguments;
    (void)arguments.insert(executableRange);
    result.push_back({ executableRange.range, { title, command, arguments } });
}

void HandleClassTest(
    const Ptr<Decl> invocationDecl, ExecutableRange executableRange, const Ptr<const MacroExpandDecl> macroExpandDecl,
    const ArkAST &ast, std::vector<CodeLens> &result)
{
    Position startPosition = {
        static_cast<unsigned int>(macroExpandDecl->begin.fileID),
        macroExpandDecl->begin.line - 1,
        macroExpandDecl->begin.column
    };
    const auto classDecl = dynamic_cast<const ClassDecl*>(invocationDecl.get());
    if (!classDecl) {
        return;
    }
    executableRange.className = classDecl->identifier;

    Position endPosition = {
        static_cast<unsigned int>(classDecl->end.fileID),
        classDecl->end.line - 1,
        classDecl->end.column
    };
    PositionUTF8ToIDE(ast.tokens, startPosition, *macroExpandDecl);
    PositionUTF8ToIDE(ast.tokens, endPosition, *classDecl);
    executableRange.range = {startPosition, endPosition};

    HandleTestResult(result, TITLE_RUN, COMMAND_TEST_RUN, executableRange);
    HandleTestResult(result, TITLE_DEBUG, COMMAND_TEST_DEBUG, executableRange);
}

void HandleFuncTest(
    const Ptr<Decl> invocationDecl, ExecutableRange executableRange, const Ptr<const MacroExpandDecl> macroExpandDecl,
    const ArkAST &ast, std::vector<CodeLens> &result)
{
    Position startPosition = {
        static_cast<unsigned int>(macroExpandDecl->begin.fileID),
        macroExpandDecl->begin.line - 1,
        macroExpandDecl->begin.column
    };
    const auto funcDecl = dynamic_cast<const FuncDecl*>(invocationDecl.get());
    if (!funcDecl) {
        return;
    }
    executableRange.functionName = funcDecl->identifier;
    executableRange.className = macroExpandDecl->GetConstInvocation()->outerDeclIdent;

    if (funcDecl->outerDecl && funcDecl->outerDecl->astKind == ASTKind::CLASS_DECL) {
        auto classDecl = dynamic_cast<const ClassDecl*>(funcDecl->outerDecl.get());
        if (classDecl) {
            executableRange.className = classDecl->identifier;
        }
    }

    Position endPosition = {
        static_cast<unsigned int>(funcDecl->end.fileID),
        funcDecl->end.line - 1,
        funcDecl->end.column
    };
    PositionUTF8ToIDE(ast.tokens, startPosition, *macroExpandDecl);
    PositionUTF8ToIDE(ast.tokens, endPosition, *funcDecl);
    executableRange.range = {startPosition, endPosition};

    HandleTestResult(result, TITLE_RUN, COMMAND_TEST_RUN, executableRange);
    HandleTestResult(result, TITLE_DEBUG, COMMAND_TEST_DEBUG, executableRange);
}

void GetTestRange(std::vector<CodeLens> &result, const ArkAST &ast)
{
    const std::string macroExpandDeclQuery = "ast_kind:macro_expand_decl";
    const auto macroExpandDeclSymbols = ast.packageInstance->ctx->searcher->Search(*ast.packageInstance->ctx,
        macroExpandDeclQuery, Sort::posDesc, {ast.file->fileHash});
    for (const auto item : macroExpandDeclSymbols) {
        if (!item) { continue; }
        const auto macroExpandDecl = dynamic_cast<const MacroExpandDecl*>(item->node.get());
        bool invalidDecl = !macroExpandDecl || !macroExpandDecl->GetConstInvocation() ||
                !macroExpandDecl->GetConstInvocation()->decl ||
                std::find(CODE_LENS_TARGET.begin(), CODE_LENS_TARGET.end(),
                          macroExpandDecl->identifier) == CODE_LENS_TARGET.end();
        if (invalidDecl) { continue; }
        ExecutableRange executableRange;
        const std::string uri = URI::URIFromAbsolutePath(CodeLensImpl::curFilePath).ToString();
        executableRange.uri = uri;
        executableRange.projectName = executableRange.packageName = macroExpandDecl->fullPackageName;
        auto invocationDecl = macroExpandDecl->GetConstInvocation()->decl.get();
        if (invocationDecl->astKind == ASTKind::CLASS_DECL) {
            HandleClassTest(invocationDecl, executableRange, macroExpandDecl, ast, result);
        } else if (invocationDecl->astKind == ASTKind::FUNC_DECL) {
            HandleFuncTest(invocationDecl, executableRange, macroExpandDecl, ast, result);
        }
    }
}

void CodeLensImpl::GetCodeLens(const ArkAST &ast, std::vector<CodeLens> &result)
{
    bool invalid = !ast.file || !ast.packageInstance;
    if (invalid) {
        return;
    }
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "CodeLensImpl::GetCodeLens in.");
    curFilePath = ast.file->filePath;

    GetTestRange(result, ast);
}
}
