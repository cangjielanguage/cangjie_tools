// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_UTILS_H
#define LSPSERVER_UTILS_H

#include <cassert>
#include <set>
#include <utility>
#include <sstream>
#include "cangjie/AST/ASTContext.h"
#include "cangjie/AST/Match.h"
#include "cangjie/Basic/StringConvertor.h"
#include "cangjie/Utils/FileUtil.h"
#include "../../json-rpc/Common.h"
#include "../../json-rpc/WorkSpaceSymbolType.h"
#include "../index/Symbol.h"
#include "../logger/Logger.h"
#include "Constants.h"
#include "FindDeclUsage.h"
#include "ItemResolverUtil.h"
#include "PositionResolver.h"

#ifdef __linux__
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>
#endif

namespace ark {
using TextEditMap = std::unordered_map<std::string, std::set<TextEdit>>;

enum class TypeCompatibility {
    INCOMPATIBLE = 0,
    IDENTICAL = 2
};

enum class CommentKind {
    NO_COMMENT = 0,
    // line comment like this comment
    LINE_COMMENT = 1,
    /* block comment like this comment */
    BLOCK_COMMENT = 2,
    /**
     * doc comment like this comment
     */
    DOC_COMMENT = 3
};

std::string GetString(const Cangjie::AST::Ty &ty);

std::string PrintTypeArgs(std::vector<Ptr<Cangjie::AST::Ty>> tyArgs, const std::pair<bool, int> isVarray = {false, 0});

CommentKind GetCommentKind(const std::string &comment);

template<typename T, typename N>
bool Is(const N *node)
{
    return dynamic_cast<const T *>(node) != nullptr;
}

TypeCompatibility CheckTypeCompatibility(const Cangjie::AST::Ty *lvalue, const Cangjie::AST::Ty *rvalue);

bool IsFuncParameterTypesIdentical(const Cangjie::AST::FuncTy &t1, const Cangjie::AST::FuncTy& t2);

bool IsMatchingCompletion(const std::string &prefix, const std::string &completionName, bool caseSensitive = false);

std::string GetSortText(double score);

std::string GetFilterText(const std::string &name, const std::string &prefix);

template<typename ActualType>
Range GetUserRange(const Cangjie::AST::Node &node)
{
    Range range;
    auto *actualType = dynamic_cast<const ActualType *>(&node);
    if (actualType != nullptr) {
        range = {
                actualType->GetFieldPos(),
                {
                        actualType->GetFieldPos().fileID,
                        actualType->GetFieldPos().line,
                        actualType->GetFieldPos().column + static_cast<int>(CountUnicodeCharacters(actualType->field))
                }
        };
    }
    return range;
}

template<typename ActualType>
Range GetMacroRange(const Cangjie::AST::Node &node)
{
    Range range;
    auto *actualType = dynamic_cast<const ActualType *>(&node);
    if (actualType != nullptr) {
        auto start = actualType->invocation.identifierPos;
        if (actualType->invocation.fullNameDotPos.size()) {
            start = actualType->invocation.fullNameDotPos.back();
            start.column++;
        }
        auto end = start;
        end.column = end.column + static_cast<int>(CountUnicodeCharacters(actualType->invocation.identifier));
        range = { start, end };
    }
    return range;
}

template<>
inline Range GetMacroRange<Cangjie::AST::MacroExpandDecl>(const Cangjie::AST::Node &node)
{
    Range range;
    auto *actualType = dynamic_cast<const Cangjie::AST::MacroExpandDecl*>(&node);
    if (actualType != nullptr) {
        auto start = actualType->GetIdentifierPos();
        if (!actualType->invocation.fullNameDotPos.empty()) {
            start = actualType->invocation.fullNameDotPos.back();
            start.column++;
        }
        auto end = start;
        end.column = start.column + static_cast<int>(CountUnicodeCharacters(actualType->identifier));
        range = { start, end };
    }
    return range;
}

Range GetNamedFuncArgRange(const Cangjie::AST::Node &node);

Range GetDeclRange(const Cangjie::AST::Decl &decl, int length);

Range GetDeclRange(const Cangjie::AST::Decl &decl);

Range GetIdentifierRange(Ptr<const Cangjie::AST::Node> node);

Range GetRefTypeRange(Ptr<const Cangjie::AST::Node> node);

bool IsZeroPosition(Ptr<const Cangjie::AST::Node> node);

bool ValidExtendIncludeGenericParam(Ptr<const Cangjie::AST::Decl> decl);

void SetRangForInterpolatedString(const Cangjie::Token &curToken, Ptr<const Cangjie::AST::Node> node, Range& range);

void SetRangForInterpolatedStrInRename(const Cangjie::Token &curToken, Ptr<const Cangjie::AST::Node> node,
    Range &range, Cangjie::Position pos);

void DealMemberParam(const std::string &curFullPkgName, std::vector<Symbol *> &syms,
                     std::vector<Ptr<Cangjie::AST::Decl> > &decls, Ptr<const Cangjie::AST::Decl> oldDecl);

std::vector<Cangjie::AST::Symbol*> SearchContext(const Cangjie::ASTContext *astContext, const std::string &query);

std::set<Ptr<Cangjie::AST::Decl> > GetInheritDecls(Ptr<const Cangjie::AST::Decl> decl);

bool IsFuncSignatureIdentical(const Cangjie::AST::FuncDecl &funcDecl1,
                              const Cangjie::AST::FuncDecl &funcDecl2);

bool IsFromSrcOrNoSrc(Ptr<const Cangjie::AST::Node> node);

bool IsFromCIMap(const std::string &fullPkgName);

bool IsFromCIMapNotInSrc(const std::string &fullPkgName);

Range GetRangeFromNode(Ptr<const Cangjie::AST::Node> p, const std::vector<Cangjie::Token> &tokens);

SymbolKind GetSymbolKind(Cangjie::AST::ASTKind astKind);

bool InValidDecl(Ptr<const Cangjie::AST::Decl> decl);

std::string GetPkgNameFromNode(Ptr<const Cangjie::AST::Node> node);

std::unordered_set<Ptr<Cangjie::AST::Node>> GetOnePkgUsers(
    const Cangjie::AST::Decl &decl, const std::string &curPkg, const std::string &curFilePath = "",
    bool isRename = false, const std::string &editPkg = "");

void SetHeadByFilePath(const std::string& filePath);

std::vector<Ptr<Cangjie::AST::Decl>> GetAimSubDecl(
    const std::string &curPkg, std::map<std::string, bool> superDecl, const std::string &editPkg = "");

int GetDotIndexPos(const std::vector<Cangjie::Token> &tokens, const int end, const int start);

void ConvertCarriageToSpace(std::string &str);

void GetConditionCompile(const nlohmann::json &initializationOptions,
                         std::unordered_map<std::string, std::string>& conditions);

void GetModuleConditionCompile(const nlohmann::json &initializationOptions,
    const std::unordered_map<std::string, std::string>& globalConditions,
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& conditions);

void GetSingleConditionCompile(const nlohmann::json &initializationOptions,
    const std::unordered_map<std::string, std::string>& globalConditions,
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& moduleConditions,
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& conditions);

void GetConditionCompilePaths(const nlohmann::json &initializationOptions, std::vector<std::string> &conditionPaths);

bool IsRelativePathByImported(const std::string &path);

bool IsFullPackageName(const std::string &pkgName);

void LowFileName(std::basic_string<char> &filePath);

std::pair<std::string, std::string> SplitFullPackage(const std::string& fullPackageName);

std::string PathWindowsToLinux(const std::string &path);

std::string LSPJoinPath(const std::string& base, const std::string& append);

std::optional<std::string> GetRelativePath(const std::string& basePath, const std::string& path);

bool IsMarkPos(Ptr<const Cangjie::AST::Node> node, Cangjie::Position pos);

bool IsResourcePos(const ArkAST &ast, Ptr<const Cangjie::AST::Node> node, Cangjie::Position pos);

std::string Digest(const std::string &pkgPath);
std::string DigestForCjo(const std::string &cjoPath);

inline bool IsGlobalOrMember(const AST::Decl& decl)
{
    if (decl.astKind == ASTKind::EXTEND_DECL) {
        return false;
    }
    return decl.TestAnyAttr(AST::Attribute::GLOBAL, AST::Attribute::IN_CLASSLIKE, AST::Attribute::IN_ENUM,
        AST::Attribute::IN_STRUCT, AST::Attribute::IN_EXTEND);
}

inline bool IsExtendDecl(const AST::Decl& decl)
{
    if (decl.astKind == ASTKind::EXTEND_DECL) {
        return true;
    }
    return false;
}

inline bool IsGlobalOrMemberOrItsParam(const AST::Decl& decl)
{
    if (decl.TestAttr(Attribute::IMPLICIT_ADD) && !decl.TestAttr(Attribute::CONSTRUCTOR)) {
        return false;
    } else if (IsGlobalOrMember(decl)) {
        return true;
    }
    auto fp = DynamicCast<const FuncParam*>(&decl);
    return fp && fp->isNamedParam;
}

inline bool isLambda(const AST::Decl& decl)
{
    if (decl.TestAttr(Attribute::IMPLICIT_ADD) && !decl.TestAttr(Attribute::CONSTRUCTOR)) {
        return false;
    }
    if (decl.astKind != ASTKind::VAR_DECL) {
        return false;
    }
    auto varDecl = DynamicCast<const VarDecl *>(&decl);
    return varDecl && varDecl->initializer && varDecl->initializer->astKind == ASTKind::LAMBDA_EXPR;
}

inline bool IsLocalFuncOrLambda(const AST::Decl& decl)
{
    if (decl.TestAttr(Attribute::IMPLICIT_ADD) && !decl.TestAttr(Attribute::CONSTRUCTOR)) {
        return false;
    }
    if (decl.astKind == ASTKind::FUNC_DECL) {
        return true;
    }
    return isLambda(decl);
}

lsp::SymbolID GetSymbolId(const Decl &decl);

uint32_t GetFileIdForDB(const std::string &fileName);

// func xxx(a:Int64,b:Varray<T>,c:(Int64)->Unit) return ["Int64","Varray<T>","(Int64)->Unit)"]
std::vector<std::string> GetFuncParamsTypeName(Ptr<const Cangjie::AST::FuncDecl> decl);

Range GetConstructorRange(const Decl &decl, const std::string identifier);

std::string GetConstructorIdentifier(const Decl &decl, bool getTargetName = true);

std::string GetVarDeclType(Ptr<const VarDecl> decl);

std::string GetStandardDeclAbsolutePath(Ptr<const Decl> decl, std::string &path);

bool IsModifierBeforeDecl(Ptr<const Decl> decl, const Position &pos);

Range GetProperRange(const Ptr<Node>& node, const std::vector<Cangjie::Token> &tokens, bool converted = true);

#ifdef _WIN32
void GetRealFileName(std::string &fileName, std::string &filePath);
#endif

std::string Trim(const std::string &str);

std::string LTrim(std::string& str);

std::string RTrim(std::string& str);

bool EndsWith(const std::string &str, const std::string &suffix);

bool RemoveFilePathExtension(const std::string &path, const std::string &extension, std::string &res);

std::string GetRealPkgNameFromPath(const std::string &str);

bool CheckIsRawIdentifier(Ptr<Node> node);

inline std::string SpliceFullPkgName(const std::string &module, const std::string &package)
{
    return module + "." + package;
}

bool InImportSpec(const File &file, Position pos);

bool IsInCjlibDir(const std::string &path);

void CategorizeFiles(
    const std::vector<std::string> &files, std::vector<std::string> &nativeFiles);

bool IsUnderPath(const std::string &basePath, const std::string &targetPath);

std::string GetSubStrBetweenSingleQuote(const std::string& str);

void GetCurFileImportedSymbolIDs(Cangjie::ImportManager *importManager, Ptr<const File> file,
    std::unordered_set<ark::lsp::SymbolID> &symbolIDs);

ark::lsp::SymbolID GetDeclSymbolID(const Decl& decl);

bool IsValidIdentifier(const std::string& identifier);

bool DeleteCharForPosition(std::string& text, int row, int column);

uint64_t GenTaskId(const std::string &packageName);

char GetSeparator();

bool IsFirstSubDir(const std::string& dir, const std::string& subDir);

int GetCurTokenInTargetTokens(const Position &pos, const std::vector<Token> &tokens, int start, int end);

std::string remove_quotes(std::string str);

using IDArray = std::vector<uint8_t>;
IDArray GetArrayFromID(uint64_t hash);

std::optional<std::string> GetSysCap(const Expr& e);

std::string GetSysCapFromDecl(const Decl &decl);

TokenKind FindPreFirstValidTokenKind(const ark::ArkAST &input, int index);
}
#endif // LSPSERVER_UTILS_H
