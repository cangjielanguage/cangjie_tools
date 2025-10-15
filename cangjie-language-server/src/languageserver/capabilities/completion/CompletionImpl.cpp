// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CompletionImpl.h"

#include <algorithm>
#include <vector>
#include "DotCompleterByParse.h"
#include "KeywordCompleter.h"
#include "NormalCompleterByParse.h"
#include "../../common/SyscapCheck.h"

#undef THIS
using namespace Cangjie;
namespace {
std::vector<Cangjie::Token> GetLineTokens(const std::vector<Cangjie::Token> &inputTokens, int line)
{
    std::vector<Cangjie::Token> res;
    for (auto &token : inputTokens) {
        if (token.Begin().line == line) {
            res.push_back(token);
        }
    }
    return res;
}

// For case: import std.xxx.{}
bool IsMultiImport(const std::vector<Cangjie::Token> &inputTokens)
{
    if (inputTokens.empty()) {
        return false;
    }
    if (inputTokens.front().kind == Cangjie::TokenKind::IMPORT) {
        auto preTokenKind = Cangjie::TokenKind::INIT;
        for (auto &token : inputTokens) {
            if (token.kind == Cangjie::TokenKind::LCURL && preTokenKind == Cangjie::TokenKind::DOT) {
                return true;
            }
            preTokenKind = token.kind;
        }
    }
    return false;
}

std::string GetMultiImportPrefix(const std::vector<Cangjie::Token> &inputTokens)
{
    std::string prefix;
    std::vector<std::string> items;
    for (size_t i = 0; inputTokens[i].kind != Cangjie::TokenKind::LCURL; ++i) {
        if (inputTokens[i].kind == Cangjie::TokenKind::IMPORT) {
            continue;
        }
        if (inputTokens[i].kind == Cangjie::TokenKind::IDENTIFIER) {
            items.push_back(inputTokens[i].Value());
        }
    }

    if (items.empty()) {
        return {};
    }

    prefix = items[0];
    for (size_t i = 1; i < items.size(); ++i) {
        prefix += "." + items[i];
    }
    return prefix;
}

bool InterpolationString(
    const ark::ArkAST &input, ark::DotCompleterByParse &dotCompleter, Cangjie::Position &pos, std::string &prefix)
{
    if (prefix.size() > 1 && prefix.find('.') != std::string::npos) {
        auto found = prefix.find_last_of('.');
        auto chainedName = prefix.substr(0, found);
        if (prefix.back() == '.') {
            prefix = ".";
        } else {
            pos.column = pos.column - static_cast<int>((prefix.size() - (found + 1)));
            prefix = prefix.substr(found + 1, (prefix.size() - (found - 1)));
        }
        dotCompleter.Complete(input, pos, chainedName);
        return true;
    }
    return false;
}

std::string GetInterpolationPrefix(const ark::ArkAST &input, const StringPart &str, Position &pos)
{
    std::string interpolation = "${";
    size_t size = interpolation.size();
    Position exprPos = {str.begin.line, str.begin.column + static_cast<int>(interpolation.size())};
    std::string value = str.value.substr(size, str.value.length() - size - 1);
    Lexer lexer = Lexer(value, input.packageInstance->diag, *input.sourceManager, exprPos);
    auto newTokens = lexer.GetTokens();
    std::vector<Token> tokenV;
    for (size_t i = 0; i < newTokens.size(); i ++) {
        auto nt = newTokens[i];
        bool isValidTy = nt.kind == TokenKind::IDENTIFIER || nt.kind == TokenKind::DOT;
        if (isValidTy) {
            tokenV.push_back(nt);
        } else {
            tokenV.clear();
        }
        if (nt.Begin() == pos && nt.kind == TokenKind::DOT) {
            pos.column = pos.column + 1;
            break;
        }
        if (nt.End() == pos) {
            if (i + 1 < newTokens.size() - 1 && newTokens[i + 1].kind == TokenKind::DOT) {
                tokenV.push_back(newTokens[i + 1]);
                pos.column = pos.column + 1;
            }
            break;
        }
    }
    std::string newPrefix;
    std::for_each(tokenV.begin(), tokenV.end(), [&newPrefix](Token t) { newPrefix += t.Value(); });
    std::cerr << "newPrefix is " << newPrefix << std::endl;
    return newPrefix;
}
}

namespace ark {
Token CompletionImpl::curToken = Token(TokenKind::INIT);
bool CompletionImpl::needImport = false;
std::unordered_set<std::string> CompletionImpl::externalImportSym = {"ohos.ark_interop_macro:Interop"};

CompletionItem CodeCompletion::Render(const std::string &sortText, const std::string &prefix) const
{
    CompletionItem item;
    item.label = label;
    item.kind = kind;
    item.detail = detail;
    item.insertText = insertText;
    item.insertTextFormat = InsertTextFormat::SNIPPET;
    if (item.kind == CompletionItemKind::CIK_MODULE || item.insertText == name || item.insertText == name + "()") {
        item.insertTextFormat = InsertTextFormat::PLAIN_TEXT;
    }
    item.filterText = GetFilterText(name, prefix);
    item.additionalTextEdits = additionalTextEdits;
    item.deprecated = deprecated;
    item.sortText = sortText;
    return item;
}

void CompletionImpl::CodeComplete(const ArkAST &input,
                                  Cangjie::Position pos,
                                  CompletionResult &result,
                                  std::string &prefix)
{
    Trace::Log("CodeComplete in.");

    // update pos fileID
    pos.fileID = input.fileID;
    // adjust position from IDE to AST
    pos = PosFromIDE2Char(pos);
    // set value for whether to execute import package
    needImport = ark::InImportSpec(*input.file, pos);
    PositionIDEToUTF8ForC(input, pos);

    auto index = input.GetCurToken(pos, 0, static_cast<int>(input.tokens.size()) - 1);
    if (index == -1) {
        return;
    }

    // Get completion prefix
    Token tok = input.tokens[static_cast<unsigned int>(index)];
    curToken = tok;
    prefix = tok.Value().substr(0, static_cast<unsigned int>(pos.column - tok.Begin().column));
    // For STRING_LITERAL, caculate accurate completino prefix.
    // If cursor isn't in interpolation, no code completion support.
    if (tok.kind == TokenKind::STRING_LITERAL || tok.kind == TokenKind::MULTILINE_STRING) {
        Lexer lexer = Lexer(input.tokens, input.packageInstance->diag, *input.sourceManager);
        auto stringParts = lexer.GetStrParts(tok);
        bool isInExpr = false;
        for (const auto &str : stringParts) {
            if (str.strKind == Cangjie::StringPart::EXPR) {
                if (str.begin <= pos && pos <= str.end) {
                    isInExpr = true;
                    prefix = GetInterpolationPrefix(input, str, pos);
                    break;
                }
            }
        }
        if (!isInExpr) {
            return;
        }
    }

    auto curLineTokens = GetLineTokens(input.tokens, pos.line);
    if (IsMultiImport(curLineTokens)) {
        prefix = '.';
    }

    // If input.semaCache is not nullptr, it's parse complete
    if (input.semaCache && input.semaCache->packageInstance && input.semaCache->packageInstance->ctx) {
        FasterComplete(input, pos, result, index, prefix);
    }
}

void CompletionImpl::FasterComplete(
    const ArkAST &input, Cangjie::Position pos, CompletionResult &result, int index, std::string &prefix)
{
    if (!input.packageInstance) { return; }
    auto &importManager =
        needImport ? input.packageInstance->importManager : input.semaCache->packageInstance->importManager;
    DotCompleterByParse dotCompleter(*(input.semaCache->packageInstance->ctx), result, importManager);

    // Completion action implement by DotCompleter.
    // Interpolation String: "${obja.b}"
    if (InterpolationString(input, dotCompleter, pos, prefix)) {
        return;
    }

    int firstTokIdxInLine = GetFirstTokenOnCurLine(input.tokens, pos.line);
    Token prevTokOfPrefix = Token(TokenKind::INIT);
    Token firstTokInLine = Token(TokenKind::INIT);
    if (firstTokIdxInLine != -1) {
        firstTokInLine = input.tokens[firstTokIdxInLine];
    }
    if (index > 0) {
        prevTokOfPrefix = input.tokens[static_cast<unsigned int>(index - 1)];
    }
    int dotIndex = GetDotIndexPos(input.tokens, index, firstTokIdxInLine);
    if (dotIndex != -1 && IsPreambleComplete(input, firstTokInLine.kind, firstTokIdxInLine)) {
        if (prevTokOfPrefix.kind == TokenKind::AS) { return; }
        auto beforeKind = prevTokOfPrefix.kind != TokenKind::COMMA && prevTokOfPrefix.kind != TokenKind::LCURL &&
                          prevTokOfPrefix.kind != TokenKind::DOT && prefix != ".";
        if (beforeKind) {
            CodeCompletion asToken;
            asToken.name = asToken.label = asToken.insertText = "as";
            asToken.kind = CompletionItemKind::CIK_KEYWORD;
            result.completions.push_back(asToken);
            return;
        }
    }

    // If the prevToken is INTEGER_LITERAL or FLOAT_LITERAL, no completion is provided.
    // If a letter is entered after the dot, completion is provided.
    // let a = 100.     <--  no completion provided
    //            ^
    // let a = 100.p    <--  completion provided
    //             ^
    if (Utils::In(prevTokOfPrefix.kind, {TokenKind::INTEGER_LITERAL, TokenKind::FLOAT_LITERAL})) {
        return;
    }

    // Import Spec: import pkg.[item]
    // Fully qualified Type: xxx.[item]
    if (prefix == ".") {
        auto chainedName = GetChainedNameComplex(input, firstTokIdxInLine, index - 1);
        auto curLineTokens = GetLineTokens(input.tokens, pos.line);
        if (IsMultiImport(curLineTokens)) {
            chainedName = chainedName.empty() ? GetMultiImportPrefix(curLineTokens)
                                              : GetMultiImportPrefix(curLineTokens) + CONSTANTS::DOT + chainedName;
        }
        dotCompleter.Complete(input, pos, chainedName);
        return;
    }

    // Import Spec: import pkg.v[item]
    // Fully qualified Type: xxx.v[item]
    if (prevTokOfPrefix.kind == TokenKind::DOT) {
        if (index > 1) {
            pos.column = prevTokOfPrefix.Begin().column + 1;
            pos.line = prevTokOfPrefix.Begin().line;
            auto chainedName = GetChainedNameComplex(input, firstTokIdxInLine, index - 2);
            dotCompleter.Complete(input, pos, chainedName);
            return;
        }
    }

    // For word wrap dot completion
    // class A {
    //     func foo() {}
    // }
    // eg: let a = A().\n <- this is Token NL
    //     fo <- this is completion position
    int offset = 2; // Offset between prefix and dot
    if (prevTokOfPrefix.kind == TokenKind::NL && index > offset) {
        size_t prevTokenIdxOfNL = static_cast<unsigned int>(index - offset);
        Token prevTokenOfNL = input.tokens[prevTokenIdxOfNL];
        if (prevTokenOfNL.kind == TokenKind::DOT) {
            pos.column = prevTokenOfNL.Begin().column + 1;
            pos.line = prevTokenOfNL.Begin().line;
            auto chainedName = GetChainedNameComplex(input, firstTokIdxInLine, index - offset);
            dotCompleter.Complete(input, pos, chainedName);
            return;
        }
    }

    // Completion action implement by NormalCompleter.
    NormalParseImpl(input, pos, result, index, prefix);
}

void CompletionImpl::AutoImportPackageComplete(const ArkAST &input, CompletionResult &result, const std::string &prefix)
{
    auto index = CompilerCangjieProject::GetInstance()->GetIndex();
    if (!index) {
        return;
    }
    if (!input.file || !input.file->package || !input.file->curPackage) {
        return;
    }
    auto pkgName = input.file->curPackage->fullPackageName;
    // get import's pos
    int lastImportLine = 0;
    for (const auto &import : input.file->imports) {
        if (!import) {
            continue;
        }
        lastImportLine = std::max(import->content.rightCurlPos.line, std::max(import->importPos.line, lastImportLine));
    }
    Position pkgPos = input.file->package->packagePos;
    if (lastImportLine == 0 && pkgPos.line > 0) {
        lastImportLine = pkgPos.line;
    }
    Position textEditStart = {input.fileID, lastImportLine, 0};
    Range textEditRange{textEditStart, textEditStart};
    auto curModule = SplitFullPackage(input.file->curPackage->fullPackageName).first;
    SyscapCheck syscap(curModule);
    index->FindImportSymsOnCompletion(std::make_pair(result.normalCompleteSymID, result.importDeclsSymID),
        pkgName, curModule, prefix,
        [&result, &textEditRange, &syscap](const std::string &pkg,
            const lsp::Symbol &sym, const lsp::CompletionItem &completionItem) {
            if (!sym.syscap.empty() && !syscap.CheckSysCap(sym.syscap)) {
                return;
            }
            CodeCompletion completion;
            std::string fullSymName = pkg + ":" + sym.name;
            if (externalImportSym.count(fullSymName)) {
                CompletionImpl::HandleExternalSymAutoImport(result, pkg, sym, completionItem, textEditRange);
                return;
            }
            auto astKind = sym.kind;
            completion.deprecated = sym.isDeprecated;
            completion.kind = ItemResolverUtil::ResolveKindByASTKind(astKind);
            completion.name = sym.name;
            completion.label = completionItem.label;
            completion.insertText = completionItem.insertText;
            completion.detail = "import " + pkg;
            TextEdit textEdit;
            textEdit.range = textEditRange;
            textEdit.newText = "import " + pkg + "." + sym.name + "\n";
            completion.additionalTextEdits = std::vector<TextEdit>{textEdit};
            completion.sortType = SortType::AUTO_IMPORT_SYM;
            result.completions.push_back(completion);
        });
}

void CompletionImpl::HandleExternalSymAutoImport(CompletionResult &result, const std::string &pkg,
    const lsp::Symbol &sym, const lsp::CompletionItem &completionItem, Range textEditRange)
{
    if (sym.name == "Interop" && pkg == "ohos.ark_interop_macro") {
        CodeCompletion completion;
        auto astKind = sym.kind;
        completion.deprecated = sym.isDeprecated;
        completion.kind = ItemResolverUtil::ResolveKindByASTKind(astKind);
        completion.name = sym.name;
        completion.label = completionItem.label;
        completion.insertText = completionItem.insertText;
        completion.detail = "import " + pkg;
        ark::TextEdit textEdit;
        textEdit.range = textEditRange;
        textEdit.newText = "import ohos.ark_interop_macro.*\nimport ohos.ark_interop.*\n";
        completion.additionalTextEdits = std::vector<TextEdit>{textEdit};
        completion.sortType = SortType::AUTO_IMPORT_SYM;
        result.completions.push_back(completion);
        return;
    }
}

void CompletionImpl::NormalParseImpl(
    const ArkAST &input, const Cangjie::Position &pos, CompletionResult &result, int index, std::string &prefix)
{
    int firstTokenIndexOnLine = GetFirstTokenOnCurLine(input.tokens, pos.line);
    TokenKind afterFirstTokenKind = TokenKind::INIT;
    TokenKind beforePrefixKind = TokenKind::INIT;
    TokenKind firstTokenKind = TokenKind::INIT;
    if (firstTokenIndexOnLine != -1) {
        firstTokenKind = input.tokens[firstTokenIndexOnLine].kind;
        if (static_cast<unsigned int>(firstTokenIndexOnLine + 1) < input.tokens.size()) {
            afterFirstTokenKind = input.tokens[firstTokenIndexOnLine + 1].kind;
        }
    }
    if (index > 0) {
        beforePrefixKind = input.tokens[static_cast<unsigned int>(index - 1)].kind;
    }

    auto &importManager =
        needImport ? input.packageInstance->importManager : input.semaCache->packageInstance->importManager;
    NormalCompleterByParse normalCompleter(result, &importManager, *(input.semaCache->packageInstance->ctx), prefix);

    // Complete package name.
    // package [name]
    // macro package [name]
    if (beforePrefixKind == TokenKind::PACKAGE) {
        normalCompleter.CompletePackageSpec(input);
        return;
    }

    // NormalComplete only need supply module name in Import Spec.
    // modifier? import [module]
    // modifier? import {[module] }
    // modifier? import {std.collection.ArrayList, [module]}
    if (IsPreamble(input, pos)) {
        auto curModule = SplitFullPackage(input.file->curPackage->fullPackageName).first;
        normalCompleter.CompleteModuleName(curModule);
        return;
    }

    // recognize typing identifier[just same as keyword] of toplevel like decls which shouldn't to be completed
    // e.g. func is, class case, enum in, ...
    // expect cases like : func param decl, enum constructor
    if (KeywordCompleter::keyWordKinds.count(curToken.kind) &&
        KeywordCompleter::declKeyWordKinds.count(FindPreFirstValidTokenKind(input, index))) {
        return;
    }

    // No code completion needed in identifier of Decl.
    if (!normalCompleter.Complete(input, pos)) {
        return;
    }

    // Complete all keywords and build-in snippets.
    KeywordCompleter::Complete(result);

    if (Options::GetInstance().IsOptionSet("disableAutoImport")) {
        return;
    }

    AutoImportPackageComplete(input, result, prefix);
}

std::string CompletionImpl::GetChainedNameComplex(const ArkAST &input, int start, int end)
{
    bool isInvalid = start > end || start < 0 || static_cast<unsigned int>(end) >= input.tokens.size();
    if (isInvalid) {
        return "";
    }

    std::string chainedName;
    std::stack<TokenKind> quoteStack = {};
    std::set<TokenKind> identifier = {TokenKind::IDENTIFIER, TokenKind::SUPER, TokenKind::THIS, TokenKind::QUEST,
                                      TokenKind::PUBLIC, TokenKind::PRIVATE, TokenKind::PROTECTED, TokenKind::OVERRIDE,
                                      TokenKind::ABSTRACT, TokenKind::OPEN, TokenKind::REDEF, TokenKind::SEALED};
    std::set<TokenKind> leftPunct = {TokenKind::LSQUARE, TokenKind::LT, TokenKind::LPAREN};
    std::set<TokenKind> rightPunct = {TokenKind::RSQUARE, TokenKind::GT, TokenKind::RPAREN};
    std::map<TokenKind, TokenKind> quoteMap = {{TokenKind::RPAREN, TokenKind::LPAREN},
                                               {TokenKind::GT, TokenKind::LT},
                                               {TokenKind::RSQUARE, TokenKind::LSQUARE}};
    size_t i;
    size_t zeroPos = 0;
    bool hasDot = true;
    bool skipQuest = false;
    for (i = static_cast<unsigned int>(end); i >= static_cast<unsigned int>(start); --i) {
        if (skipQuest) {
            skipQuest = false;
            continue;
        }
        if (rightPunct.find(input.tokens[i].kind) != rightPunct.end()) {
            quoteStack.push(input.tokens[i].kind);
        } else if (leftPunct.find(input.tokens[i].kind) != leftPunct.end()) {
            if (quoteStack.empty() || quoteMap[quoteStack.top()] != input.tokens[i].kind) {
                continue;
            }
            quoteStack.pop();
            continue;
        }
        if (!quoteStack.empty()) {
            continue;
        }
        if (!hasDot && input.tokens[i].kind == TokenKind::DOT) {
            hasDot = true;
            (void)chainedName.insert(zeroPos, input.tokens[i].Value());
        } else if (hasDot && identifier.find(input.tokens[i].kind) != identifier.end()) {
            (void)chainedName.insert(zeroPos, input.tokens[i].Value());
            if (input.tokens[i].kind == TokenKind::QUEST && i - 1 >= 0) {
                (void)chainedName.insert(zeroPos, input.tokens[i - 1].Value());
                skipQuest = true;
            }
            hasDot = false;
        } else {
            break;
        }
    }
    if (!quoteStack.empty()) {
        return "";
    }
    return chainedName;
}

bool CompletionImpl::IsPreambleComplete(const ArkAST &input,
                                        const TokenKind firstTokenKind,
                                        const int firstTokenIndexOnLine)
{
    bool keywordFlag = firstTokenKind == TokenKind::IMPORT || firstTokenKind == TokenKind::PACKAGE;
    if (keywordFlag) {
        return true;
    }
    if (firstTokenKind == TokenKind::PUBLIC &&
        static_cast<unsigned int>(firstTokenIndexOnLine + 1) < input.tokens.size() &&
        input.tokens[firstTokenIndexOnLine + 1].kind == TokenKind::IMPORT) {
        return true;
    }
    return false;
}

bool CompletionImpl::IsPreamble(const ArkAST &input, Cangjie::Position pos)
{
    for (const auto &im : input.file->imports) {
        if (pos <= im->end) {
            return true;
        }
    }
    return false;
}
} // namespace ark
