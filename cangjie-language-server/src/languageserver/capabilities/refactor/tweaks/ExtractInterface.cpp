// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <algorithm>
#include <cctype>
#include <cstring>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include "../../../CompilerCangjieProject.h"
#include "../../../capabilities/reference/FindReferencesImpl.h"
#include "../../../common/FindDeclUsage.h"
#include "../../../common/ItemResolverUtil.h"
#include "../../../common/Utils.h"
#include "../TweakRule.h"
#include "../TweakUtils.h"
#include "ExtractInterface.h"
#include "cangjie/Utils/FileUtil.h"

namespace ark {
using namespace Cangjie;
using namespace AST;

constexpr size_t JSON_ESCAPE_RESERVE_EXTRA = 4;
constexpr const char *INTERFACE_MEMBER_INDENT = "    ";
constexpr const char *INHERITED_MEMBER_PREFIX = "<:";
constexpr const char *FILE_URI_PREFIX = "file:/";
constexpr const char *DOC_COMMENT_PREFIX = "/**";
// LCOV_EXCL_BR_START
enum class TargetKind {
    CLASS,
    STRUCT,
    INTERFACE,
    ENUM,
    EXTEND
};

struct TargetDecl {
    TargetKind kind;
    Cangjie::AST::InheritableDecl *inheritableDecl = nullptr;
    Cangjie::AST::ClassDecl *classDecl = nullptr;
    Cangjie::AST::StructDecl *structDecl = nullptr;
    Cangjie::AST::InterfaceDecl *interfaceDecl = nullptr;
    Cangjie::AST::EnumDecl *enumDecl = nullptr;
    Cangjie::AST::ExtendDecl *extendDecl = nullptr;
    std::string name;
};

struct InterfaceInfo {
    struct MemberMeta {
        std::string signature;
        bool isStatic = false;
        bool isMut = false;
        bool isOperator = false;
        std::string visibility;
    };

    std::string name;
    std::string genericParams;
    std::string genericWhereClause;
    std::vector<std::string> inheritedTypes;
    std::vector<std::string> members;
    std::vector<MemberMeta> metas;
    std::unordered_map<std::string, std::string> implBodies;
    std::unordered_map<std::string, std::string> memberComments;
};

struct SelectedEditsRequest {
    const Tweak::Selection &sel;
    const std::unordered_set<std::string> &chosen;
    std::vector<TextEdit> &edits;
    bool addOverride = false;
};

struct TypeReplacementContext {
    const Tweak::Selection &sel;
    Cangjie::AST::Decl &targetDecl;
    std::string interfaceName;
    std::string implementationClassName;
    std::string targetPath;
    std::unordered_set<std::string> allowedMemberNames;
    std::map<std::string, std::vector<TextEdit>> &applyEdits;
};

struct ApplyContext {
    const Tweak::Selection &sel;
    const TargetDecl &target;
    Tweak::Effect effect;
    InterfaceInfo info;
    std::string sourceUri;
    std::string targetPath;
    std::string implementationClassName;
    std::unordered_set<std::string> chosen;
    std::unordered_set<std::string> selectedInheritedTypes;
    bool renameOriginalClass = false;
    bool hasExplicitInterfaceName = false;
    bool hasSelectedMembersOption = false;
    bool withImplementation = true;
};

// LCOV_EXCL_BR_STOP
void CollectTypeReferenceReplacementEdits(TypeReplacementContext &context);

std::string GetVisibility(const Cangjie::AST::Decl &decl)
{
    using Attr = Cangjie::AST::Attribute;
    if (decl.TestAttr(Attr::PUBLIC)) {
        return "public";
    }
    if (decl.TestAttr(Attr::PROTECTED)) {
        return "protected";
    }
    if (decl.TestAttr(Attr::INTERNAL)) {
        return "internal";
    }
    if (decl.TestAttr(Attr::PRIVATE)) {
        return "private";
    }
    return "public";
}

std::string EscapeJsonString(const std::string &input)
{
    std::string escaped;
    escaped.reserve(input.size() + JSON_ESCAPE_RESERVE_EXTRA);
    for (char ch : input) {
        if (ch == '\"') {
            escaped += "\\\"";
        } else if (ch == '\\') {
            escaped += "\\\\";
        } else {
            escaped.push_back(ch);
        }
    }
    return escaped;
}

std::string ResolveFuncSignature(const FuncDecl &func, SourceManager *sm)
{
    std::string signature = ItemResolverUtil::ResolveSignatureByNode(func, sm);
    if (func.funcBody && func.funcBody->retType) {
        std::string ret = ItemResolverUtil::FetchTypeString(*func.funcBody->retType);
        if (!ret.empty()) {
            signature += ": " + ret;
        }
    }
    return signature;
}
// LCOV_EXCL_START
std::string ExtractFuncBodyText(const FuncDecl &func, SourceManager *sm)
{
    if (!sm) {
        return "";
    }
    std::string whole = sm->GetContentBetween(func.GetBegin(), func.GetEnd());
    size_t brace = whole.find('{');
    if (brace != std::string::npos) {
        size_t rightBrace = whole.rfind('}');
        if (rightBrace != std::string::npos && rightBrace >= brace) {
            return whole.substr(brace, rightBrace - brace + 1);
        }
        return "";
    }
    size_t arrow = whole.find("=>");
    if (arrow != std::string::npos) {
        return whole.substr(arrow);
    }
    return "";
}

void AppendCommentGroupText(std::string &result, const std::vector<CommentGroup> &groupList)
{
    for (const auto &group : groupList) {
        for (const auto &comment : group.cms) {
            std::string text = comment.info.Value();
            if (text.empty()) {
                continue;
            }
            auto first = text.find_first_not_of(" \t\r\n");
            if (first != std::string::npos && text.compare(first, std::strlen(DOC_COMMENT_PREFIX),
                DOC_COMMENT_PREFIX) == 0) {
                continue;
            }
            if (!result.empty() && result.back() != '\n') {
                result.push_back('\n');
            }
            result += text;
            if (!result.empty() && result.back() != '\n') {
                result.push_back('\n');
            }
        }
    }
}

std::string ExtractDeclCommentText(const Cangjie::AST::Decl &decl)
{
    const auto &groups = decl.comments;
    if (groups.IsEmpty()) {
        return "";
    }

    std::string result;
    AppendCommentGroupText(result, groups.leadingComments);
    AppendCommentGroupText(result, groups.innerComments);
    AppendCommentGroupText(result, groups.trailingComments);
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}
// LCOV_EXCL_STOP
// LCOV_EXCL_BR_START
void AppendMemberJson(std::string &membersJson,
                      bool &first,
                      const std::string &signature,
                      bool isStatic,
                      const std::string &visibility)
{
    if (!first) {
        membersJson.push_back(',');
    }
    first = false;
    membersJson += "{\"signature\":\"" + EscapeJsonString(signature) + "\",\"isStatic\":" +
        std::string(isStatic ? "true" : "false") + ",\"visibility\":\"" + EscapeJsonString(visibility) + "\"}";
}
// LCOV_EXCL_BR_STOP
std::optional<TargetDecl> BuildClassTargetDecl(Cangjie::AST::ClassDecl *decl)
{
    if (decl == nullptr) {
        return std::nullopt;
    }
    return TargetDecl{
        TargetKind::CLASS,
        static_cast<Cangjie::AST::InheritableDecl *>(decl),
        decl,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        decl->identifier
    };
}

std::optional<TargetDecl> BuildStructTargetDecl(Cangjie::AST::StructDecl *decl)
{
    if (decl == nullptr) {
        return std::nullopt;
    }
    return TargetDecl{
        TargetKind::STRUCT,
        static_cast<Cangjie::AST::InheritableDecl *>(decl),
        nullptr,
        decl,
        nullptr,
        nullptr,
        nullptr,
        decl->identifier
    };
}

std::optional<TargetDecl> BuildInterfaceTargetDecl(Cangjie::AST::InterfaceDecl *decl)
{
    if (decl == nullptr) {
        return std::nullopt;
    }
    return TargetDecl{
        TargetKind::INTERFACE,
        static_cast<Cangjie::AST::InheritableDecl *>(decl),
        nullptr,
        nullptr,
        decl,
        nullptr,
        nullptr,
        decl->identifier
    };
}

std::optional<TargetDecl> BuildEnumTargetDecl(Cangjie::AST::EnumDecl *decl)
{
    if (decl == nullptr) {
        return std::nullopt;
    }
    return TargetDecl{
        TargetKind::ENUM,
        static_cast<Cangjie::AST::InheritableDecl *>(decl),
        nullptr,
        nullptr,
        nullptr,
        decl,
        nullptr,
        decl->identifier
    };
}
// LCOV_EXCL_BR_START
std::string ResolveExtendTargetName(const Cangjie::AST::ExtendDecl &decl)
{
    std::string extendName;
    if (decl.extendedType) {
        extendName = Trim(decl.extendedType->ToString());
        auto genericPos = extendName.find('<');
        if (genericPos != std::string::npos) {
            extendName = extendName.substr(0, genericPos);
        }
    }
    if (extendName.empty() && decl.extendedType && decl.extendedType->GetTy()) {
        extendName = Trim(decl.extendedType->GetTy()->name);
    }
    return extendName;
}

std::optional<TargetDecl> BuildExtendTargetDecl(Cangjie::AST::ExtendDecl *decl)
{
    if (decl == nullptr) {
        return std::nullopt;
    }
    return TargetDecl{
        TargetKind::EXTEND,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        decl,
        ResolveExtendTargetName(*decl)
    };
}

std::optional<TargetDecl> BuildTargetDecl(Cangjie::AST::Decl *decl)
{
    if (auto *classDecl = dynamic_cast<Cangjie::AST::ClassDecl *>(decl)) {
        return BuildClassTargetDecl(classDecl);
    }
    if (auto *structDecl = dynamic_cast<Cangjie::AST::StructDecl *>(decl)) {
        return BuildStructTargetDecl(structDecl);
    }
    if (auto *interfaceDecl = dynamic_cast<Cangjie::AST::InterfaceDecl *>(decl)) {
        return BuildInterfaceTargetDecl(interfaceDecl);
    }
    if (auto *enumDecl = dynamic_cast<Cangjie::AST::EnumDecl *>(decl)) {
        return BuildEnumTargetDecl(enumDecl);
    }
    if (auto *extendDecl = dynamic_cast<Cangjie::AST::ExtendDecl *>(decl)) {
        return BuildExtendTargetDecl(extendDecl);
    }
    return std::nullopt;
}

bool ContainsSelectionRange(const Cangjie::AST::Decl &decl, const Range &selection)
{
    return decl.begin <= selection.start && decl.end >= selection.end;
}

std::optional<TargetDecl> FindTargetDeclInDeclTree(Cangjie::AST::Decl &decl, const Range &selection)
{
    if (!ContainsSelectionRange(decl, selection)) {
        return std::nullopt;
    }

    for (auto &member : decl.GetMemberDecls()) {
        if (!member) {
            continue;
        }
        auto nestedTarget = FindTargetDeclInDeclTree(*member.get(), selection);
        if (nestedTarget.has_value()) {
            return nestedTarget;
        }
    }
    return BuildTargetDecl(&decl);
}

std::optional<TargetDecl> ResolveTargetDecl(const Tweak::Selection &sel)
{
    if (auto top = sel.selectionTree.TopDecl()) {
        auto *topDecl = dynamic_cast<Cangjie::AST::Decl *>(top.get());
        if (topDecl) {
            auto target = BuildTargetDecl(topDecl);
            if (target.has_value()) {
                return target;
            }
        }
    }

    const auto *root = sel.selectionTree.root();
    if (!root || !root->node) {
        return std::nullopt;
    }

    auto *decl = dynamic_cast<Cangjie::AST::Decl *>(root->node.get());
    while (decl) {
        auto target = BuildTargetDecl(decl);
        if (target.has_value()) {
            return target;
        }
        decl = decl->outerDecl;
    }

    if (sel.arkAst && sel.arkAst->file) {
        for (auto &topDecl : sel.arkAst->file->decls) {
            if (!topDecl) {
                continue;
            }
            auto target = FindTargetDeclInDeclTree(*topDecl.get(), sel.range);
            if (target.has_value()) {
                return target;
            }
        }
    }
    return std::nullopt;
}

bool ParseBoolOption(const std::map<std::string, std::string> &options,
                     const std::string &key,
                     bool defaultValue)
{
    auto it = options.find(key);
    if (it == options.end()) {
        return defaultValue;
    }
    return it->second == "true";
}

std::string ResolveImplementationClassName(const std::map<std::string, std::string> &options,
                                           const std::string &fallback)
{
    auto it = options.find("implementationClassName");
    if (it != options.end()) {
        std::string implName = Trim(it->second);
        if (!implName.empty()) {
            return implName;
        }
    }
    return Trim(fallback);
}

template <typename DeclType>
std::string GetGenericListForDecl(DeclType *decl)
{
    if (decl == nullptr) {
        return "";
    }
    return ItemResolverUtil::GetGenericParamByDecl(decl->generic.get());
}
// LCOV_EXCL_BR_STOP
template <typename DeclType>
std::string GetGenericWhereClauseForDecl(DeclType *decl, SourceManager *sm)
{
// LCOV_EXCL_BR_START
    if (decl == nullptr || decl->generic == nullptr || sm == nullptr) {
        return "";
    }
    const auto &constraints = decl->generic->genericConstraints;
    if (constraints.empty()) {
        return "";
    }

    std::string clause;
    bool first = true;
    for (const auto &constraint : constraints) {
        if (!constraint) {
            continue;
        }
        std::string text = Trim(sm->GetContentBetween(constraint->GetBegin(), constraint->GetEnd()));
        if (text.empty()) {
            continue;
        }
        if (first) {
            clause += " where ";
            first = false;
        } else {
            clause += ", ";
        }
        if (text.rfind("where", 0) == 0) {
            text = Trim(text.substr(std::string("where").size()));
        }
        clause += text;
    }
    return clause;
// LCOV_EXCL_BR_STOP
}

std::string ResolveTargetGenericParams(const TargetDecl &target)
{
    switch (target.kind) {
        case TargetKind::CLASS:
            return GetGenericListForDecl(target.classDecl);
        case TargetKind::STRUCT:
            return GetGenericListForDecl(target.structDecl);
        case TargetKind::INTERFACE:
            return GetGenericListForDecl(target.interfaceDecl);
        case TargetKind::ENUM:
            return GetGenericListForDecl(target.enumDecl);
        case TargetKind::EXTEND:
            return GetGenericListForDecl(target.extendDecl);
        default:
            return "";
    }
}

std::string ResolveTargetGenericWhereClause(const TargetDecl &target, SourceManager *sm)
{
    switch (target.kind) {
        case TargetKind::CLASS:
            return GetGenericWhereClauseForDecl(target.classDecl, sm);
        case TargetKind::STRUCT:
            return GetGenericWhereClauseForDecl(target.structDecl, sm);
        case TargetKind::INTERFACE:
            return GetGenericWhereClauseForDecl(target.interfaceDecl, sm);
        case TargetKind::ENUM:
            return GetGenericWhereClauseForDecl(target.enumDecl, sm);
        case TargetKind::EXTEND:
            return GetGenericWhereClauseForDecl(target.extendDecl, sm);
        default:
            return "";
    }
}

std::string JoinTypes(const std::vector<std::string> &types)
{
    std::ostringstream oss;
    for (size_t i = 0; i < types.size(); ++i) {
        if (i != 0) {
            oss << " & ";
        }
        oss << types[i];
    }
    return oss.str();
}

void AddSelectedMemberSignature(std::unordered_set<std::string> &chosen, const std::string &signature)
{
    std::string sig = Trim(signature);
    if (sig.empty()) {
        return;
    }
    chosen.insert(sig);
    chosen.insert(TweakUtils::NormalizeSignature(sig));
}

bool ParseSelectedMembersFromJson(const std::string &raw, std::unordered_set<std::string> &chosen)
{
    if (raw.empty() || (raw.front() != '[' && raw.front() != '{')) {
        return false;
    }
    nlohmann::json json = nlohmann::json::parse(raw, nullptr, false);
    if (json.is_discarded() || !json.is_array()) {
        return false;
    }
    for (auto &entry : json) {
        if (!entry.is_string()) {
            continue;
        }
        AddSelectedMemberSignature(chosen, entry.get<std::string>());
    }
    return true;
}

void ParseSelectedMembersFromText(const std::string &raw, std::unordered_set<std::string> &chosen)
{
    std::vector<std::string> entries = Split(raw, "\n");
    if (entries.size() == 1) {
        auto csvEntries = Split(raw, ",");
        if (csvEntries.size() > 1) {
            entries = std::move(csvEntries);
        }
    }
    for (const auto &entry : entries) {
        AddSelectedMemberSignature(chosen, entry);
    }
}

std::unordered_set<std::string> ParseSelectedMembers(const std::map<std::string, std::string> &options)
{
    std::unordered_set<std::string> chosen;
    auto it = options.find("selectedMembers");
    if (it == options.end()) {
        return chosen;
    }

    const std::string &raw = it->second;
    if (!ParseSelectedMembersFromJson(raw, chosen)) {
        ParseSelectedMembersFromText(raw, chosen);
    }
    return chosen;
}

std::string NormalizeUriString(const std::string &uri)
{
    if (uri.empty()) {
        return uri;
    }
    if (uri.rfind(FILE_URI_PREFIX, 0) == 0) {
        return URI::URIFromAbsolutePath(FileStore::NormalizePath(URI::Resolve(uri))).ToString();
    }
    return URI::URIFromAbsolutePath(FileStore::NormalizePath(uri)).ToString();
}
// LCOV_EXCL_START
std::optional<Location> ParseSelectedTypeReferenceEntry(const nlohmann::json &entry)
{
    if (!entry.is_object() || !entry.contains("uri") || !entry.contains("start") || !entry.contains("end")) {
        return std::nullopt;
    }
    const auto &start = entry["start"];
    const auto &end = entry["end"];
    if (!start.is_object() || !end.is_object()) {
        return std::nullopt;
    }
    bool hasStart = start.contains("line") && start.contains("character");
    bool hasEnd = end.contains("line") && end.contains("character");
    if (!hasStart || !hasEnd) {
        return std::nullopt;
    }

    Location location;
    location.uri.file = NormalizeUriString(entry.value("uri", ""));
    location.range.start = {0, start.value("line", 0), start.value("character", 0)};
    location.range.end = {0, end.value("line", 0), end.value("character", 0)};
    return location;
}
// LCOV_EXCL_STOP
bool IsSameSelectedTypeReference(const Location &lhs, const Location &rhs)
{
    return NormalizeUriString(lhs.uri.file) == NormalizeUriString(rhs.uri.file) &&
           lhs.range.start.line == rhs.range.start.line &&
           lhs.range.start.column == rhs.range.start.column &&
           lhs.range.end.line == rhs.range.end.line &&
           lhs.range.end.column == rhs.range.end.column;
}

std::optional<std::string> ParseInheritedMemberSignature(const std::string &signature)
{
    std::string text = Trim(signature);
    if (text.rfind(INHERITED_MEMBER_PREFIX, 0) != 0) {
        return std::nullopt;
    }
    std::string typeName = Trim(text.substr(std::strlen(INHERITED_MEMBER_PREFIX)));
    if (typeName.empty()) {
        return std::nullopt;
    }
    return typeName;
}

std::string NormalizeTypeNameForCompare(std::string name)
{
    name = Trim(name);
    if (name.empty()) {
        return name;
    }
    size_t genericPos = name.find('<');
    if (genericPos != std::string::npos) {
        name = name.substr(0, genericPos);
    }
    size_t dotPos = name.rfind('.');
    if (dotPos != std::string::npos && dotPos + 1 < name.size()) {
        name = name.substr(dotPos + 1);
    }
    return Trim(name);
}
// LCOV_EXCL_START
std::string ResolveInheritedTypeText(const Ptr<Type> &inherited, SourceManager *sm)
{
    if (!inherited) {
        return "";
    }
    std::string typeName;
    if (sm) {
        typeName = Trim(sm->GetContentBetween(inherited->GetBegin(), inherited->GetEnd()));
    }
    if (typeName.empty() && inherited->GetTy()) {
        typeName = Trim(inherited->GetTy()->String());
    }
    return typeName;
}

bool IsInterfaceInheritedType(const Ptr<Type> &inherited)
{
    if (!inherited || !inherited->GetTy() || inherited->GetTy()->IsObject()) {
        return false;
    }
    auto decl = Ty::GetDeclPtrOfTy(inherited->GetTy());
    return DynamicCast<InterfaceDecl *>(decl) != nullptr;
}

std::vector<InterfaceDecl *> CollectDirectInheritedInterfaceDecls(InheritableDecl &decl)
{
    std::vector<InterfaceDecl *> inheritedInterfaces;
    std::unordered_set<InterfaceDecl *> seen;
    for (auto &inherited : decl.inheritedTypes) {
        if (!inherited || !inherited->GetTy() || inherited->GetTy()->IsObject()) {
            continue;
        }
        auto interfaceDecl = DynamicCast<InterfaceDecl *>(Ty::GetDeclPtrOfTy(inherited->GetTy()));
        if (!interfaceDecl || !seen.insert(interfaceDecl).second) {
            continue;
        }
        inheritedInterfaces.push_back(interfaceDecl);
    }
    return inheritedInterfaces;
}
// LCOV_EXCL_STOP
std::unordered_set<std::string> ParseSelectedInheritedMemberTypes(const std::unordered_set<std::string> &chosen)
{
    std::unordered_set<std::string> selectedTypes;
    for (const auto &member : chosen) {
        auto parsedType = ParseInheritedMemberSignature(member);
        if (!parsedType.has_value()) {
            continue;
        }
        selectedTypes.insert(NormalizeTypeNameForCompare(*parsedType));
    }
    return selectedTypes;
}

// LCOV_EXCL_START
void CollectInheritedTypesForExtract(const Cangjie::AST::InheritableDecl &decl,
                                     const Tweak::Selection &sel,
                                     std::vector<std::string> &inheritedTypes)
{
    SourceManager *sm = sel.arkAst ? sel.arkAst->sourceManager : nullptr;
    if (!sm) {
        return;
    }
    std::unordered_set<std::string> seen;
    for (const auto &inherited : decl.inheritedTypes) {
        if (!inherited) {
            continue;
        }
        if (inherited->GetTy() && inherited->GetTy()->IsObject()) {
            continue;
        }
        std::string typeName = Trim(sm->GetContentBetween(inherited->GetBegin(), inherited->GetEnd()));
        if (typeName.empty()) {
            continue;
        }
        if (seen.insert(typeName).second) {
            inheritedTypes.push_back(std::move(typeName));
        }
    }
}

void CollectInheritedTypesForExtract(const Cangjie::AST::ExtendDecl &decl,
                                     const Tweak::Selection &sel,
                                     std::vector<std::string> &inheritedTypes)
{
    SourceManager *sm = sel.arkAst ? sel.arkAst->sourceManager : nullptr;
    if (!sm) {
        return;
    }
    std::unordered_set<std::string> seen;
    for (const auto &inherited : decl.inheritedTypes) {
        if (!inherited) {
            continue;
        }
        if (inherited->GetTy() && inherited->GetTy()->IsObject()) {
            continue;
        }
        std::string typeName = Trim(sm->GetContentBetween(inherited->GetBegin(), inherited->GetEnd()));
        if (typeName.empty()) {
            continue;
        }
        if (seen.insert(typeName).second) {
            inheritedTypes.push_back(std::move(typeName));
        }
    }
}

std::string NormalizeTargetPath(const std::map<std::string, std::string> &options)
{
    auto it = options.find("targetPath");
    if (it == options.end()) {
        return "";
    }
    std::string targetPath = Trim(it->second);
    if (targetPath.empty()) {
        return "";
    }
    if (targetPath.rfind(FILE_URI_PREFIX, 0) == 0) {
        targetPath = URI::Resolve(targetPath);
    }
    return FileStore::NormalizePath(targetPath);
}

std::optional<std::pair<Cangjie::Position, Cangjie::Position>> FindTransferableLeadingCommentsRange(
    const Cangjie::AST::Decl &decl)
{
    if (decl.comments.leadingComments.empty()) {
        return std::nullopt;
    }

    Cangjie::Position begin = Cangjie::INVALID_POSITION;
    Cangjie::Position end = Cangjie::INVALID_POSITION;
    for (const auto &group : decl.comments.leadingComments) {
        for (const auto &comment : group.cms) {
            std::string text = comment.info.Value();
            auto first = text.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) {
                continue;
            }
            if (first != std::string::npos && text.compare(first, std::strlen(DOC_COMMENT_PREFIX),
                DOC_COMMENT_PREFIX) == 0) {
                continue;
            }

            const auto &cBegin = comment.info.Begin();
            const auto &cEnd = comment.info.End();
            if (begin.IsZero() || cBegin < begin) {
                begin = cBegin;
            }
            if (end.IsZero() || end < cEnd) {
                end = cEnd;
            }
        }
    }
    if (begin.IsZero() || end.IsZero()) {
        return std::nullopt;
    }
    return std::make_pair(begin, end);
}

std::string ToLowerAscii(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}
// LCOV_EXCL_STOP
std::string ToSnakeCaseFileName(const std::string &name)
{
    std::string result;
    result.reserve(name.size());
    bool previousIsUnderscore = false;
    bool previousIsLowerOrDigit = false;
    for (size_t i = 0; i < name.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(name[i]);
        if (std::isalnum(ch) == 0) {
            if (!result.empty() && !previousIsUnderscore) {
                result.push_back('_');
                previousIsUnderscore = true;
            }
            previousIsLowerOrDigit = false;
            continue;
        }
        bool isUpper = std::isupper(ch) != 0;
        bool isLower = std::islower(ch) != 0;
        bool isDigit = std::isdigit(ch) != 0;
        bool nextIsLower = i + 1 < name.size() && std::islower(static_cast<unsigned char>(name[i + 1])) != 0;
        if (isUpper && !result.empty() && !previousIsUnderscore && (previousIsLowerOrDigit || nextIsLower)) {
            result.push_back('_');
        }
        result.push_back(static_cast<char>(std::tolower(ch)));
        previousIsUnderscore = false;
        previousIsLowerOrDigit = isLower || isDigit;
    }
    while (!result.empty() && result.back() == '_') {
        result.pop_back();
    }
    return result.empty() ? ToLowerAscii(name) : result;
}

std::string ResolveTargetFilePath(const std::string &targetPath, const std::string &interfaceName)
{
    if (targetPath.empty()) {
        return "";
    }
    if (Cangjie::FileUtil::GetFileExtension(targetPath) == "cj" || interfaceName.empty()) {
        return targetPath;
    }
    return FileStore::NormalizePath(
        Cangjie::FileUtil::JoinPath(targetPath, ToSnakeCaseFileName(interfaceName) + ".cj"));
}

bool IsValidPackageName(const std::string &packageName)
{
    if (packageName.empty()) {
        return false;
    }
    return packageName != "default" &&
           packageName.find('/') == std::string::npos &&
           packageName.find('\\') == std::string::npos &&
           packageName.find(':') == std::string::npos;
}
// LCOV_EXCL_BR_START
std::string ResolveTargetPackageName(const std::string &targetPath)
{
    if (targetPath.empty()) {
        return "";
    }
    auto *project = CompilerCangjieProject::GetInstance();
    if (!project) {
        return "";
    }

    std::string targetDir = Cangjie::FileUtil::GetDirPath(targetPath);
    std::string fullPkgName = project->GetFullPkgByDir(targetDir);
    if (fullPkgName.empty()) {
        fullPkgName = project->GetFullPkgName(targetPath);
    }
    if (fullPkgName.empty()) {
        return "";
    }
    std::string packageName = project->GetRealPackageName(fullPkgName);
    return IsValidPackageName(packageName) ? packageName : "";
}

void CollectClassInterfaceInheritedTypesForRename(const Cangjie::AST::ClassDecl &decl,
                                                  const Tweak::Selection &sel,
                                                  std::vector<std::string> &inheritedTypes)
{
    SourceManager *sm = sel.arkAst ? sel.arkAst->sourceManager : nullptr;
    if (!sm) {
        return;
    }

    std::unordered_set<std::string> seen;
    for (const auto &inherited : decl.inheritedTypes) {
        if (!IsInterfaceInheritedType(inherited)) {
            continue;
        }
        std::string typeName = ResolveInheritedTypeText(inherited, sm);
        if (typeName.empty()) {
            continue;
        }
        if (seen.insert(typeName).second) {
            inheritedTypes.push_back(std::move(typeName));
        }
    }
}

void CollectInterfaceInheritedTypesForExtract(const Cangjie::AST::InheritableDecl &decl,
                                              const Tweak::Selection &sel,
                                              const std::unordered_set<std::string> &selectedInheritedTypes,
                                              std::vector<std::string> &inheritedTypes)
{
    SourceManager *sm = sel.arkAst ? sel.arkAst->sourceManager : nullptr;
    if (!sm) {
        return;
    }

    std::unordered_set<std::string> seen;
    for (const auto &inherited : decl.inheritedTypes) {
        if (!IsInterfaceInheritedType(inherited)) {
            continue;
        }
        std::string typeName = ResolveInheritedTypeText(inherited, sm);
        if (typeName.empty()) {
            continue;
        }
        std::string normalized = NormalizeTypeNameForCompare(typeName);
        if (!selectedInheritedTypes.count(normalized)) {
            continue;
        }
        if (seen.insert(normalized).second) {
            inheritedTypes.push_back(std::move(typeName));
        }
    }
}

std::string NormalizePathForCompare(std::string path)
{
    path = FileStore::NormalizePath(path);
    std::replace(path.begin(), path.end(), '\\', '/');
#ifdef _WIN32
    std::transform(path.begin(), path.end(), path.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
#endif
    while (!path.empty() && path.back() == '/') {
        path.pop_back();
    }
    return path;
}

std::string BuildImportFullSymWithoutAlias(const ImportContent &importContent)
{
    std::stringstream ss;
    for (size_t i = 0; i < importContent.prefixPaths.size(); ++i) {
        ss << importContent.prefixPaths[i];
        if (i + 1 < importContent.prefixPaths.size()) {
            ss << ".";
        }
    }
    if (importContent.kind != ImportKind::IMPORT_MULTI) {
        if (!importContent.prefixPaths.empty()) {
            ss << ".";
        }
        ss << importContent.identifier.Val();
    }
    return ss.str();
}

bool HasImportForInterface(const File &file, const std::string &fullSym, const std::string &pkgName)
{
    const std::string pkgAll = pkgName + ".*";
    for (const auto &importSpec : file.imports) {
        if (!importSpec) {
            continue;
        }
        const auto &content = importSpec->content;
        if (content.kind == ImportKind::IMPORT_MULTI) {
            continue;
        }
        std::string importSym = BuildImportFullSymWithoutAlias(content);
        if (importSym == fullSym || importSym == pkgAll) {
            return true;
        }
    }
    return false;
}
// LCOV_EXCL_BR_STOP
// LCOV_EXCL_START
std::optional<TextEdit> BuildImportInterfaceEdit(const Tweak::Selection &sel,
                                                 const std::string &targetPath,
                                                 const std::string &interfaceName)
{
    if (!sel.arkAst || !sel.arkAst->file || interfaceName.empty() || targetPath.empty()) {
        return std::nullopt;
    }

    std::string sourceDir = NormalizePathForCompare(Cangjie::FileUtil::GetDirPath(sel.arkAst->file->filePath));
    std::string targetDir = NormalizePathForCompare(Cangjie::FileUtil::GetDirPath(targetPath));
    if (sourceDir.empty() || targetDir.empty() || sourceDir == targetDir) {
        return std::nullopt;
    }

    std::string targetPkgName = ResolveTargetPackageName(targetPath);
    if (targetPkgName.empty()) {
        return std::nullopt;
    }
    std::string importSym = targetPkgName + "." + interfaceName;
    if (HasImportForInterface(*sel.arkAst->file, importSym, targetPkgName)) {
        return std::nullopt;
    }

    Position pos = Cangjie::INVALID_POSITION;
    if (sel.arkAst->file->package) {
        pos = FindLastImportPos(*sel.arkAst->file);
    } else if (!sel.arkAst->file->decls.empty() && sel.arkAst->file->decls.front()) {
        pos = sel.arkAst->file->decls.front()->GetBegin();
    } else {
        pos = sel.arkAst->file->begin;
    }
    Range insertRange = {pos, pos};
    insertRange = TransformFromChar2IDE(insertRange);
    return TextEdit{insertRange, "import " + importSym + "\n"};
}
// LCOV_EXCL_STOP

std::optional<TextEdit> BuildImportInterfaceEditForFile(const File &sourceFile,
                                                        const std::string &targetPath,
                                                        const std::string &interfaceName)
{
    if (interfaceName.empty() || targetPath.empty()) {
        return std::nullopt;
    }

    std::string sourceDir = NormalizePathForCompare(Cangjie::FileUtil::GetDirPath(sourceFile.filePath));
    std::string targetDir = NormalizePathForCompare(Cangjie::FileUtil::GetDirPath(targetPath));
    if (sourceDir.empty() || targetDir.empty() || sourceDir == targetDir) {
        return std::nullopt;
    }

    std::string targetPkgName = ResolveTargetPackageName(targetPath);
    if (targetPkgName.empty()) {
        return std::nullopt;
    }
    std::string importSym = targetPkgName + "." + interfaceName;
    if (HasImportForInterface(sourceFile, importSym, targetPkgName)) {
        return std::nullopt;
    }

    Position pos = Cangjie::INVALID_POSITION;
    if (sourceFile.package) {
        pos = FindLastImportPos(sourceFile);
    } else if (!sourceFile.decls.empty() && sourceFile.decls.front()) {
        pos = sourceFile.decls.front()->GetBegin();
    } else {
        pos = sourceFile.begin;
    }
    Range insertRange = {pos, pos};
    insertRange = TransformFromChar2IDE(insertRange);
    return TextEdit{insertRange, "import " + importSym + "\n"};
}

std::optional<Range> GetTypeReferenceRange(const Type &type)
{
    if (auto qualifiedType = DynamicCast<const QualifiedType*>(&type)) {
        Position pos = {qualifiedType->GetBegin().fileID, qualifiedType->GetBegin().line,
            qualifiedType->end.column - static_cast<int>(qualifiedType->field.Val().size())};
        return Range{pos, {pos.fileID, pos.line,
            pos.column + static_cast<int>(CountUnicodeCharacters(qualifiedType->field))}};
    }
    if (auto refType = DynamicCast<const RefType*>(&type)) {
        Position pos = refType->ref.identifier.Begin();
        if (pos.IsZero()) {
            return std::nullopt;
        }
        return Range{pos, {pos.fileID, pos.line,
            pos.column + static_cast<int>(CountUnicodeCharacters(refType->ref.identifier))}};
    }
    return std::nullopt;
}

std::unordered_set<std::string> BuildAllowedMemberNames(const std::vector<std::string> &members)
{
    std::unordered_set<std::string> names;
    for (const auto &member : members) {
        auto parenPos = member.find('(');
        if (parenPos == std::string::npos) {
            continue;
        }
        std::string name = Trim(member.substr(0, parenPos));
        if (!name.empty()) {
            names.insert(name);
        }
    }
    return names;
}

bool IsTypeReferenceMatch(const Type &type, const Decl &targetDecl, const Range &referenceRange)
{
    auto target = type.GetTarget();
    if (!target || !CheckDeclEqual(*target, targetDecl)) {
        return false;
    }
    auto typeRange = GetTypeReferenceRange(type);
    return typeRange.has_value() && referenceRange.start <= typeRange->start && typeRange->end <= referenceRange.end;
}

std::optional<Range> GetNameReferenceRange(const NameReferenceExpr &refExpr)
{
    Position begin = Cangjie::INVALID_POSITION;
    size_t nameLength = 0;
    if (auto ref = DynamicCast<const RefExpr*>(&refExpr)) {
        begin = ref->GetIdentifierPos();
        nameLength = CountUnicodeCharacters(ref->ref.identifier);
    } else if (auto memberAccess = DynamicCast<const MemberAccess*>(&refExpr)) {
        begin = memberAccess->GetFieldPos();
        nameLength = CountUnicodeCharacters(memberAccess->field);
    }
    if (begin.IsZero()) {
        return std::nullopt;
    }
    Range exprRange{begin, begin};
    exprRange.end.column += static_cast<int>(nameLength);
    return exprRange;
}
// LCOV_EXCL_BR_START
bool IsMatchingNonTypeReference(const Node &node, const Decl &targetDecl, const Range &referenceRange)
{
    auto refExpr = DynamicCast<const NameReferenceExpr*>(&node);
    if (!refExpr) {
        return false;
    }
    bool targetsDecl = false;
    if (auto ref = DynamicCast<const RefExpr*>(refExpr)) {
        auto target = ref->ref.target;
        targetsDecl = target && CheckDeclEqual(*target, targetDecl);
    } else if (auto memberAccess = DynamicCast<const MemberAccess*>(refExpr)) {
        auto target = memberAccess->target;
        targetsDecl = target && CheckDeclEqual(*target, targetDecl);
    }
    if (!targetsDecl) {
        return false;
    }
    auto exprRange = GetNameReferenceRange(*refExpr);
    return exprRange.has_value() && exprRange->start == referenceRange.start && exprRange->end == referenceRange.end;
}

bool IsReferenceRangeTypeOnly(const Node &node, const Decl &targetDecl, const Range &referenceRange)
{
    bool foundTypeMatch = false;
    bool foundNonTypeRefMatch = false;
    ConstWalker(&node, [&foundTypeMatch, &foundNonTypeRefMatch, &targetDecl, &referenceRange](Ptr<const Node> child) {
        if (!child) {
            return VisitAction::WALK_CHILDREN;
        }
        if ((child->GetBegin() > referenceRange.start || referenceRange.start > child->GetEnd()) &&
            (child->GetBegin() > referenceRange.end || referenceRange.end > child->GetEnd())) {
            return VisitAction::WALK_CHILDREN;
        }
        if (auto type = DynamicCast<const Type*>(child.get())) {
            if (IsTypeReferenceMatch(*type, targetDecl, referenceRange)) {
                foundTypeMatch = true;
                return VisitAction::WALK_CHILDREN;
            }
        }
        if (IsMatchingNonTypeReference(*child.get(), targetDecl, referenceRange)) {
            foundNonTypeRefMatch = true;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return foundTypeMatch && !foundNonTypeRefMatch;
}

bool IsParamTypeReferenceRange(const FuncParam &param, const Range &referenceRange)
{
    if (!param.type) {
        return false;
    }
    auto typeRange = GetTypeReferenceRange(*param.type);
    return typeRange.has_value() && typeRange->start == referenceRange.start && typeRange->end == referenceRange.end;
}

bool IsFuncParamUsedInBody(const FuncParam &param, const FuncBody &funcBody)
{
    if (!funcBody.body) {
        return false;
    }
    bool used = false;
    ConstWalker(funcBody.body.get(), [&used, &param](Ptr<const Node> child) {
        if (used || !child) {
            return VisitAction::STOP_NOW;
        }
        auto refExpr = DynamicCast<const RefExpr*>(child.get());
        if (!refExpr) {
            return VisitAction::WALK_CHILDREN;
        }
        auto target = refExpr->ref.target;
        if (target && CheckDeclEqual(*target, param)) {
            used = true;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return used;
}

bool UsesDisallowedMembers(const Decl &decl, const std::unordered_set<std::string> &allowedMemberNames)
{
    if (allowedMemberNames.empty()) {
        return false;
    }

    bool disallowed = false;
    ConstWalker(&decl, [&decl, &allowedMemberNames, &disallowed](Ptr<const Node> child) {
        if (disallowed || !child) {
            return VisitAction::STOP_NOW;
        }
        auto memberAccess = DynamicCast<const MemberAccess*>(child.get());
        if (!memberAccess || !memberAccess->baseExpr) {
            return VisitAction::WALK_CHILDREN;
        }
        auto baseRef = DynamicCast<const RefExpr*>(memberAccess->baseExpr.get());
        if (!baseRef || !baseRef->ref.target || !CheckDeclEqual(*baseRef->ref.target, decl)) {
            return VisitAction::WALK_CHILDREN;
        }
        if (!allowedMemberNames.count(memberAccess->field.Val())) {
            disallowed = true;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return disallowed;
}

bool ShouldKeepConcreteTypeReference(const Node &node,
                                     const Range &referenceRange,
                                     const std::unordered_set<std::string> &allowedMemberNames)
{
    bool keepConcrete = false;
    ConstWalker(&node, [&referenceRange, &allowedMemberNames, &keepConcrete](Ptr<const Node> child) {
        if (keepConcrete || !child) {
            return VisitAction::STOP_NOW;
        }
        auto varDecl = DynamicCast<const VarDeclAbstract*>(child.get());
        auto decl = DynamicCast<const Decl*>(child.get());
        if (!varDecl || !decl || !varDecl->type) {
            return VisitAction::WALK_CHILDREN;
        }
        auto typeRange = GetTypeReferenceRange(*varDecl->type);
        if (!typeRange.has_value() || typeRange->start != referenceRange.start ||
            typeRange->end != referenceRange.end) {
            return VisitAction::WALK_CHILDREN;
        }
        keepConcrete = UsesDisallowedMembers(*decl, allowedMemberNames);
        return VisitAction::STOP_NOW;
    }).Walk();
    return keepConcrete;
}

bool ShouldExcludeTypeReferenceReplacement(const Node &node, const Range &referenceRange)
{
    bool shouldExclude = false;
    ConstWalker(&node, [&shouldExclude, &referenceRange](Ptr<const Node> child) {
        if (shouldExclude || !child) {
            return VisitAction::STOP_NOW;
        }
        auto funcParam = DynamicCast<const FuncParam*>(child.get());
        if (!funcParam || !IsParamTypeReferenceRange(*funcParam, referenceRange)) {
            return VisitAction::WALK_CHILDREN;
        }
        auto outerFunc = DynamicCast<const FuncDecl*>(funcParam->outerDecl);
        if (!outerFunc || !outerFunc->funcBody) {
            return VisitAction::STOP_NOW;
        }
        shouldExclude = IsFuncParamUsedInBody(*funcParam, *outerFunc->funcBody);
        return VisitAction::STOP_NOW;
    }).Walk();
    return shouldExclude;
}
// LCOV_EXCL_BR_STOP
// LCOV_EXCL_START
char PreviousNonWhitespaceChar(const std::string &text)
{
    for (auto it = text.rbegin(); it != text.rend(); ++it) {
        if (!std::isspace(static_cast<unsigned char>(*it))) {
            return *it;
        }
    }
    return '\0';
}

char NextNonWhitespaceChar(const std::string &text)
{
    for (char ch : text) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            return ch;
        }
    }
    return '\0';
}

bool IsLikelyTypeReferenceText(const ArkAST &ast, const Range &referenceRange)
{
    if (!ast.sourceManager || !ast.file) {
        return false;
    }

    std::string prefix = ast.sourceManager->GetContentBetween(ast.file->GetBegin(), referenceRange.start);
    std::string suffix = ast.sourceManager->GetContentBetween(referenceRange.end, ast.file->GetEnd());
    char left = PreviousNonWhitespaceChar(prefix);
    char right = NextNonWhitespaceChar(suffix);

    auto isLeftTypeContext = [](char ch) {
        return ch == ':' || ch == '<' || ch == '&' || ch == ',' || ch == '(';
    };
    auto isRightTypeContext = [](char ch) {
        return ch == '>' || ch == '&' || ch == ',' || ch == ')' || ch == '=' || ch == '{';
    };

    return right != '(' && (isLeftTypeContext(left) || isRightTypeContext(right));
}

bool IsLikelyImplementationClassReferenceText(const ArkAST &ast, const Range &referenceRange)
{
    if (!ast.sourceManager || !ast.file) {
        return false;
    }

    std::string suffix = ast.sourceManager->GetContentBetween(referenceRange.end, ast.file->GetEnd());
    return NextNonWhitespaceChar(suffix) == '(';
}
// LCOV_EXCL_STOP
// LCOV_EXCL_BR_START
std::optional<TextEdit> BuildTypeReplacementEditForLocation(const Location &location,
                                                            const TypeReplacementContext &context,
                                                            std::optional<TextEdit> &importEdit)
{
    auto *project = CompilerCangjieProject::GetInstance();
    if (!project) {
        return std::nullopt;
    }

    std::string path = FileStore::NormalizePath(URI::Resolve(location.uri.file));
    SetHeadByFilePath(path);
    ArkAST *refAst = project->GetArkAST(path);
    if (!refAst || !refAst->file) {
        return std::nullopt;
    }

    Range refRange = TransformFromIDE2Char(location.range);
    PositionIDEToUTF8(refAst->tokens, refRange.start, *refAst->file);
    PositionIDEToUTF8(refAst->tokens, refRange.end, *refAst->file);
    bool found = false;
    bool shouldExclude = false;
    ConstWalker(refAst->file.get(), [&found, &shouldExclude, &context, &refRange]
        (Ptr<const Node> node) {
        if (found || !node) {
            return VisitAction::STOP_NOW;
        }
        if ((node->GetBegin() > refRange.start || refRange.start > node->GetEnd()) &&
            (node->GetBegin() > refRange.end || refRange.end > node->GetEnd())) {
            return VisitAction::WALK_CHILDREN;
        }
        if (IsReferenceRangeTypeOnly(*node, context.targetDecl, refRange)) {
            found = true;
            shouldExclude = ShouldExcludeTypeReferenceReplacement(*node, refRange) ||
                            ShouldKeepConcreteTypeReference(*node, refRange, context.allowedMemberNames);
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    if (!found && !IsLikelyTypeReferenceText(*refAst, refRange)) {
        return std::nullopt;
    }
    if (shouldExclude) {
        return std::nullopt;
    }

    std::string normalizedTargetPath = FileStore::NormalizePath(context.targetPath);
    if (!normalizedTargetPath.empty() && normalizedTargetPath != path) {
        importEdit = BuildImportInterfaceEditForFile(*refAst->file, normalizedTargetPath, context.interfaceName);
    }
    return TextEdit{location.range, context.interfaceName};
}

std::optional<TextEdit> BuildImplementationClassReplacementEditForLocation(const Location &location,
                                                                           const TypeReplacementContext &context)
{
    if (context.implementationClassName.empty()) {
        return std::nullopt;
    }

    auto *project = CompilerCangjieProject::GetInstance();
    if (!project) {
        return std::nullopt;
    }

    std::string path = FileStore::NormalizePath(URI::Resolve(location.uri.file));
    SetHeadByFilePath(path);
    ArkAST *refAst = project->GetArkAST(path);
    if (!refAst || !refAst->file) {
        return std::nullopt;
    }

    Range refRange = TransformFromIDE2Char(location.range);
    PositionIDEToUTF8(refAst->tokens, refRange.start, *refAst->file);
    PositionIDEToUTF8(refAst->tokens, refRange.end, *refAst->file);
    if (!IsLikelyImplementationClassReferenceText(*refAst, refRange)) {
        return std::nullopt;
    }

    bool found = false;
    ConstWalker(refAst->file.get(), [&found, &context, &refRange](Ptr<const Node> node) {
        if (found || !node) {
            return VisitAction::STOP_NOW;
        }
        if ((node->GetBegin() > refRange.start || refRange.start > node->GetEnd()) &&
            (node->GetBegin() > refRange.end || refRange.end > node->GetEnd())) {
            return VisitAction::WALK_CHILDREN;
        }
        if (IsMatchingNonTypeReference(*node, context.targetDecl, refRange)) {
            found = true;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    if (!found) {
        return std::nullopt;
    }
    return TextEdit{location.range, context.implementationClassName};
}

std::optional<TextEdit> BuildProtectedToPublicEdit(const FuncDecl &func, SourceManager *sm)
{
    if (!sm || !func.TestAttr(Cangjie::AST::Attribute::PROTECTED)) {
        return std::nullopt;
    }

    Cangjie::Position headerBegin = func.GetBegin();
    Cangjie::Position nameBegin = func.identifier.Begin();
    std::string header = sm->GetContentBetween(headerBegin, nameBegin);
    size_t pos = header.find("protected");
    if (pos == std::string::npos) {
        return std::nullopt;
    }

    auto isIdentifierChar = [](char ch) {
        return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
    };
    bool leftBoundary = (pos == 0) || !isIdentifierChar(header[pos - 1]);
    size_t endPos = pos + std::string("protected").size();
    bool rightBoundary = endPos >= header.size() || !isIdentifierChar(header[endPos]);
    if (!leftBoundary || !rightBoundary) {
        return std::nullopt;
    }

    Cangjie::Position start = headerBegin;
    size_t offset = 0;
    while (offset < pos) {
        size_t newline = header.find('\n', offset);
        if (newline == std::string::npos || newline >= pos) {
            int advance = static_cast<int>(CountUnicodeCharacters(header.substr(offset, pos - offset)));
            start.column += advance;
            break;
        }
        int advance = static_cast<int>(CountUnicodeCharacters(header.substr(offset, newline - offset)));
        start.column += advance;
        start.line += 1;
        start.column = 1;
        offset = newline + 1;
    }

    Cangjie::Position end = start;
    end.column += static_cast<int>(CountUnicodeCharacters(std::string("protected")));
    Range range{start, end};
    range = TransformFromChar2IDE(range);
    return TextEdit{range, "public"};
}
// LCOV_EXCL_BR_STOP
std::optional<TextEdit> BuildInternalToPublicEdit(const FuncDecl &func, SourceManager *sm)
{
    if (!sm || !func.TestAttr(Cangjie::AST::Attribute::INTERNAL)) {
        return std::nullopt;
    }

    Cangjie::Position headerBegin = func.GetBegin();
    Cangjie::Position nameBegin = func.identifier.Begin();
    std::string header = sm->GetContentBetween(headerBegin, nameBegin);
    size_t pos = TweakUtils::FindTokenBoundaryPos(header, "internal");
    if (pos == std::string::npos) {
        return std::nullopt;
    }

    Cangjie::Position start = TweakUtils::PositionAtOffset(headerBegin, header, pos);
    Cangjie::Position end = TweakUtils::PositionAtOffset(headerBegin, header, pos + std::string("internal").size());
    Range range{start, end};
    range = TransformFromChar2IDE(range);
    return TextEdit{range, "public"};
}

std::optional<TextEdit> BuildDefaultVisibilityToPublicEdit(const FuncDecl &func, SourceManager *sm)
{
    if (!sm) {
        return std::nullopt;
    }
    Cangjie::Position headerBegin = func.GetBegin();
    Cangjie::Position nameBegin = func.identifier.Begin();
    std::string header = sm->GetContentBetween(headerBegin, nameBegin);

    size_t funcPos = TweakUtils::FindTokenBoundaryPos(header, "func");
    if (funcPos == std::string::npos) {
        return std::nullopt;
    }
    size_t staticPos = TweakUtils::FindTokenBoundaryPos(header, "static");
    size_t overridePos = TweakUtils::FindTokenBoundaryPos(header, "override");
    size_t insertTokenPos = funcPos;
    if (overridePos != std::string::npos && overridePos < insertTokenPos) {
        insertTokenPos = overridePos;
    }
    if (staticPos != std::string::npos && staticPos < insertTokenPos) {
        insertTokenPos = staticPos;
    }

    // Use textual visibility detection so `func test()` can still be upgraded to `public func test()`
    // even when semantic attrs treat default visibility as internal.
    if (TweakUtils::FindTokenBoundaryPos(header, "public") != std::string::npos ||
        TweakUtils::FindTokenBoundaryPos(header, "protected") != std::string::npos ||
        TweakUtils::FindTokenBoundaryPos(header, "internal") != std::string::npos ||
        TweakUtils::FindTokenBoundaryPos(header, "private") != std::string::npos) {
        return std::nullopt;
    }

    Cangjie::Position insertPos = TweakUtils::PositionAtOffset(headerBegin, header, insertTokenPos);

    Range range{insertPos, insertPos};
    range = TransformFromChar2IDE(range);
    return TextEdit{range, "public "};
}

std::optional<TextEdit> BuildRemoveVisibilityEdit(const FuncDecl &func, SourceManager *sm)
{
    if (!sm) {
        return std::nullopt;
    }
    Cangjie::Position headerBegin = func.GetBegin();
    Cangjie::Position nameBegin = func.identifier.Begin();
    std::string header = sm->GetContentBetween(headerBegin, nameBegin);

    std::vector<std::string> visTokens = {"public", "protected", "internal", "private"};
    size_t visPos = std::string::npos;
    size_t visLen = 0;
    for (const auto &token : visTokens) {
        size_t pos = TweakUtils::FindTokenBoundaryPos(header, token);
        if (pos == std::string::npos) {
            continue;
        }
        if (visPos == std::string::npos || pos < visPos) {
            visPos = pos;
            visLen = token.size();
        }
    }
    if (visPos == std::string::npos) {
        return std::nullopt;
    }

    size_t visEnd = visPos + visLen;
    while (visEnd < header.size() && (header[visEnd] == ' ' || header[visEnd] == '\t')) {
        ++visEnd;
    }

    Cangjie::Position start = headerBegin;
    TweakUtils::AdvancePosition(start, header, 0, visPos);
    Cangjie::Position end = start;
    TweakUtils::AdvancePosition(end, header, visPos, visEnd);

    Range range{start, end};
    range = TransformFromChar2IDE(range);
    return TextEdit{range, ""};
}

std::optional<TextEdit> BuildOverrideBeforeFuncEdit(const FuncDecl &func, SourceManager *sm)
{
    if (!sm) {
        return std::nullopt;
    }
    Cangjie::Position headerBegin = func.GetBegin();
    Cangjie::Position nameBegin = func.identifier.Begin();
    std::string header = sm->GetContentBetween(headerBegin, nameBegin);

    size_t funcPos = TweakUtils::FindTokenBoundaryPos(header, "func");
    if (funcPos == std::string::npos) {
        return std::nullopt;
    }
    size_t operatorPos = TweakUtils::FindTokenBoundaryPos(header, "operator");
    size_t insertTokenPos = (operatorPos != std::string::npos && operatorPos < funcPos) ? operatorPos : funcPos;
    size_t overridePos = TweakUtils::FindTokenBoundaryPos(header, "override");
    if (overridePos != std::string::npos && overridePos < insertTokenPos) {
        return std::nullopt;
    }

    Cangjie::Position insertPos = TweakUtils::PositionAtOffset(headerBegin, header, insertTokenPos);

    Range range{insertPos, insertPos};
    range = TransformFromChar2IDE(range);
    return TextEdit{range, "override "};
}

std::optional<TextEdit> BuildRedefAfterStaticEdit(const FuncDecl &func, SourceManager *sm)
{
    if (!sm) {
        return std::nullopt;
    }
    Cangjie::Position headerBegin = func.GetBegin();
    Cangjie::Position nameBegin = func.identifier.Begin();
    std::string header = sm->GetContentBetween(headerBegin, nameBegin);

    size_t funcPos = TweakUtils::FindTokenBoundaryPos(header, "func");
    size_t staticPos = TweakUtils::FindTokenBoundaryPos(header, "static");
    if (funcPos == std::string::npos || staticPos == std::string::npos || staticPos > funcPos) {
        return std::nullopt;
    }

    size_t redefPos = TweakUtils::FindTokenBoundaryPos(header, "redef");
    if (redefPos != std::string::npos && redefPos > staticPos && redefPos < funcPos) {
        return std::nullopt;
    }

    size_t insertTokenPos = staticPos + std::string("static").size();
    while (insertTokenPos < header.size() && std::isspace(static_cast<unsigned char>(header[insertTokenPos]))) {
        ++insertTokenPos;
    }

    Cangjie::Position insertPos = TweakUtils::PositionAtOffset(headerBegin, header, insertTokenPos);

    Range range{insertPos, insertPos};
    range = TransformFromChar2IDE(range);
    return TextEdit{range, "redef "};
}

size_t FindHeaderClauseInsertOffset(const std::string &headerText)
{
    size_t wherePos = TweakUtils::FindTokenBoundaryPos(headerText, "where");
    if (wherePos != std::string::npos) {
        size_t insertOffset = wherePos;
        while (insertOffset > 0 && std::isspace(static_cast<unsigned char>(headerText[insertOffset - 1]))) {
            --insertOffset;
        }
        return insertOffset;
    }

    size_t bodyPos = headerText.find('{');
    if (bodyPos != std::string::npos) {
        size_t insertOffset = bodyPos;
        while (insertOffset > 0 && std::isspace(static_cast<unsigned char>(headerText[insertOffset - 1]))) {
            --insertOffset;
        }
        return insertOffset;
    }
    return headerText.size();
}

std::optional<Cangjie::Position> FindHeaderClauseInsertPosition(const Cangjie::AST::Decl &decl,
                                                                SourceManager *sm)
{
    if (!sm) {
        return std::nullopt;
    }
    std::string declText = sm->GetContentBetween(decl.GetBegin(), decl.GetEnd());
    if (declText.empty()) {
        return std::nullopt;
    }

    size_t insertOffset = FindHeaderClauseInsertOffset(declText);
    return TweakUtils::PositionAtOffset(decl.GetBegin(), declText, insertOffset);
}

TextEdit BuildInsertAtDeclTail(const Cangjie::AST::Decl &decl, SourceManager *sm, const std::string &appendText)
{
    TextEdit tailEdit;
    auto insertPos = FindHeaderClauseInsertPosition(decl, sm);
    if (!insertPos.has_value()) {
        std::string sig = ItemResolverUtil::ResolveInsertByNode(decl, sm, false);
        int length = static_cast<int>(CountUnicodeCharacters(sig));
        if (length <= 0) {
            length = static_cast<int>(CountUnicodeCharacters(decl.identifier.Val()));
        }
        Range anchor = GetDeclRange(decl, length);
        insertPos = anchor.end;
    }

    Range insertRange = {*insertPos, *insertPos};
    insertRange = TransformFromChar2IDE(insertRange);
    tailEdit.range = insertRange;
    tailEdit.newText = appendText;
    return tailEdit;
}
// LCOV_EXCL_START
bool HasRealInheritedType(const Cangjie::AST::InheritableDecl &decl)
{
    for (auto &ty : decl.inheritedTypes) {
        if (!ty) {
            continue;
        }
        if (!ty->GetTy() || !ty->GetTy()->IsObject()) {
            return true;
        }
    }
    return false;
}
// LCOV_EXCL_STOP
// LCOV_EXCL_BR_START
template <typename TypeDecl>
void CollectMembersFromType(TypeDecl &decl, const Tweak::Selection &sel, InterfaceInfo &info)
{
    SourceManager *sm = sel.arkAst ? sel.arkAst->sourceManager : nullptr;
    auto &memberDecls = decl.GetMemberDecls();
    for (auto &member : memberDecls) {
        if (!member || member->astKind != ASTKind::FUNC_DECL) {
            continue;
        }
        auto *func = dynamic_cast<FuncDecl *>(member.get().get());
        if (!func || func->identifier.Val() == "init") {
            continue;
        }
        std::string signature = ResolveFuncSignature(*func, sm);
        bool isStatic = func->TestAttr(Cangjie::AST::Attribute::STATIC);
        info.members.push_back(signature);
        info.metas.push_back({
            signature,
            isStatic,
            func->TestAttr(Cangjie::AST::Attribute::MUT),
            func->TestAttr(Cangjie::AST::Attribute::OPERATOR),
            GetVisibility(*func)
        });

        std::string comments = ExtractDeclCommentText(*func);
        if (!comments.empty()) {
            info.memberComments[signature] = std::move(comments);
        }
        if (func->TestAttr(Cangjie::AST::Attribute::PRIVATE) || func->TestAttr(Cangjie::AST::Attribute::STATIC)) {
            std::string body = ExtractFuncBodyText(*func, sm);
            if (!body.empty()) {
                info.implBodies[signature] = std::move(body);
            }
        }
    }
}

template <typename TypeDecl>
size_t CountMembersFromType(TypeDecl &decl)
{
    size_t count = 0;
    for (auto &member : decl.GetMemberDecls()) {
        if (!member || member->astKind != ASTKind::FUNC_DECL) {
            continue;
        }
        auto *func = dynamic_cast<FuncDecl *>(member.get().get());
        if (func && func->identifier.Val() != "init") {
            ++count;
        }
    }
    return count;
}

size_t CountExtractableMembers(const TargetDecl &target)
{
    switch (target.kind) {
        case TargetKind::CLASS:
            return CountMembersFromType(*target.classDecl);
        case TargetKind::STRUCT:
            return CountMembersFromType(*target.structDecl);
        case TargetKind::INTERFACE:
            return CountMembersFromType(*target.interfaceDecl);
        case TargetKind::ENUM:
            return CountMembersFromType(*target.enumDecl);
        case TargetKind::EXTEND:
            return CountMembersFromType(*target.extendDecl);
        default:
            return 0;
    }
}
// LCOV_EXCL_BR_STOP
class ExtractInterfaceSelectionRule : public TweakRule {
public:
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto target = ResolveTargetDecl(sel);
        if (!target.has_value()) {
            extraOptions.insert(std::make_pair(
                "ErrorCode",
                std::to_string(static_cast<int>(ExtractInterface::ExtractInterfaceError::INVALID_TARGET))));
            return false;
        }

        std::vector<std::string> inheritedTypeMembers;
        if (target->inheritableDecl) {
            CollectInheritedTypesForExtract(*target->inheritableDecl, sel, inheritedTypeMembers);
        } else if (target->extendDecl) {
            CollectInheritedTypesForExtract(*target->extendDecl, sel, inheritedTypeMembers);
        }

        if (CountExtractableMembers(*target) == 0 && inheritedTypeMembers.empty()) {
            extraOptions.insert(std::make_pair(
                "ErrorCode",
                std::to_string(static_cast<int>(ExtractInterface::ExtractInterfaceError::EMPTY_METHODS))));
            return false;
        }
        return true;
    }
};
// LCOV_EXCL_START
void AddPrivateMemberRemovalEdit(const FuncDecl &func, SourceManager *sm, std::vector<TextEdit> &edits)
{
    Cangjie::Position begin = func.GetBegin();
    auto commentRange = FindTransferableLeadingCommentsRange(func);
    if (commentRange.has_value()) {
        std::string gapText = sm ? sm->GetContentBetween(commentRange->second, func.GetBegin()) : "";
        bool whitespaceOnly = std::all_of(gapText.begin(), gapText.end(), [](unsigned char ch) {
            return std::isspace(ch) != 0;
        });
        if (whitespaceOnly) {
            begin = commentRange->first;
        }
    }
    Range removeRange = {begin, func.GetEnd()};
    removeRange = TransformFromChar2IDE(removeRange);
    edits.push_back({removeRange, ""});
}
// LCOV_EXCL_STOP
bool AddInterfaceMemberEdits(const FuncDecl &func,
                             SourceManager *sm,
                             const SelectedEditsRequest &request,
                             bool isStatic)
{
    auto removeVisibilityEdit = BuildRemoveVisibilityEdit(func, sm);
    if (!removeVisibilityEdit.has_value()) {
        return false;
    }
    removeVisibilityEdit->newText = isStatic ? "" : "override ";
    request.edits.push_back(std::move(*removeVisibilityEdit));
    return !isStatic;
}

bool AddConcreteVisibilityEdits(const FuncDecl &func,
                                SourceManager *sm,
                                const SelectedEditsRequest &request,
                                bool isStatic)
{
    auto protectedEdit = BuildProtectedToPublicEdit(func, sm);
    if (protectedEdit.has_value()) {
        request.edits.push_back(std::move(*protectedEdit));
        return false;
    }

    auto internalEdit = BuildInternalToPublicEdit(func, sm);
    if (internalEdit.has_value()) {
        request.edits.push_back(std::move(*internalEdit));
        return false;
    }

    auto defaultVisibilityEdit = BuildDefaultVisibilityToPublicEdit(func, sm);
    if (!defaultVisibilityEdit.has_value()) {
        return false;
    }
    if (isStatic) {
        defaultVisibilityEdit->newText = "public ";
    } else if (request.addOverride && BuildOverrideBeforeFuncEdit(func, sm).has_value()) {
        defaultVisibilityEdit->newText = "public override ";
    }
    request.edits.push_back(std::move(*defaultVisibilityEdit));
    return true;
}

bool AddVisibilityEdits(const FuncDecl &func,
                        SourceManager *sm,
                        const SelectedEditsRequest &request,
                        bool fromInterface,
                        bool isStatic)
{
    if (!fromInterface) {
        return AddConcreteVisibilityEdits(func, sm, request, isStatic);
    }

    auto defaultVisibilityEdit = BuildDefaultVisibilityToPublicEdit(func, sm);
    if (!defaultVisibilityEdit.has_value()) {
        return false;
    }
    defaultVisibilityEdit->newText = "override ";
    request.edits.push_back(std::move(*defaultVisibilityEdit));
    return true;
}

void AddOverrideOrRedefEdit(const FuncDecl &func,
                            SourceManager *sm,
                            const SelectedEditsRequest &request,
                            bool allowRedef,
                            bool handledDefaultVisibility)
{
    bool isStatic = func.TestAttr(Cangjie::AST::Attribute::STATIC);
    if (isStatic && allowRedef) {
        auto redefEdit = BuildRedefAfterStaticEdit(func, sm);
        if (redefEdit.has_value()) {
            request.edits.push_back(std::move(*redefEdit));
        }
        return;
    }
    if (!request.addOverride || handledDefaultVisibility) {
        return;
    }
    auto overrideEdit = BuildOverrideBeforeFuncEdit(func, sm);
    if (overrideEdit.has_value()) {
        request.edits.push_back(std::move(*overrideEdit));
    }
}

void CollectSelectedMemberEdits(FuncDecl &func,
                                SourceManager *sm,
                                const SelectedEditsRequest &request,
                                bool fromInterface,
                                bool allowRedef)
{
    bool isStatic = func.TestAttr(Cangjie::AST::Attribute::STATIC);
    if (func.TestAttr(Cangjie::AST::Attribute::PRIVATE)) {
        AddPrivateMemberRemovalEdit(func, sm, request.edits);
        return;
    }

    if (fromInterface && AddInterfaceMemberEdits(func, sm, request, isStatic)) {
        return;
    }

    bool handledDefaultVisibility = AddVisibilityEdits(func, sm, request, fromInterface, isStatic);
    AddOverrideOrRedefEdit(func, sm, request, allowRedef, handledDefaultVisibility);
}

template <typename TypeDecl>
void CollectSelectedEditsFromType(TypeDecl &decl,
                                  const SelectedEditsRequest &request,
                                  bool fromInterface = false,
                                  bool allowRedef = true)
{
// LCOV_EXCL_BR_START
    SourceManager *sm = request.sel.arkAst ? request.sel.arkAst->sourceManager : nullptr;
    auto &memberDecls = decl.GetMemberDecls();
    for (auto &member : memberDecls) {
        if (!member || member->astKind != ASTKind::FUNC_DECL) {
            continue;
        }
        auto *func = dynamic_cast<FuncDecl *>(member.get().get());
        if (!func || func->identifier.Val() == "init") {
            continue;
        }
        std::string signature = ResolveFuncSignature(*func, sm);
        if (!request.chosen.count(signature) && !request.chosen.count(TweakUtils::NormalizeSignature(signature))) {
            continue;
        }
        CollectSelectedMemberEdits(*func, sm, request, fromInterface, allowRedef);
    }
// LCOV_EXCL_BR_STOP
}

void CollectMembersFromTarget(const TargetDecl &target, const Tweak::Selection &sel, InterfaceInfo &info)
{
    switch (target.kind) {
        case TargetKind::CLASS:
            CollectMembersFromType(*target.classDecl, sel, info);
            return;
        case TargetKind::STRUCT:
            CollectMembersFromType(*target.structDecl, sel, info);
            return;
        case TargetKind::INTERFACE:
            CollectMembersFromType(*target.interfaceDecl, sel, info);
            return;
        case TargetKind::ENUM:
            CollectMembersFromType(*target.enumDecl, sel, info);
            return;
        case TargetKind::EXTEND:
            CollectMembersFromType(*target.extendDecl, sel, info);
            return;
        default:
            return;
    }
}

void CollectInheritedTypesFromTarget(const TargetDecl &target,
                                     const Tweak::Selection &sel,
                                     std::vector<std::string> &inheritedTypes)
{
    if (target.extendDecl) {
        CollectInheritedTypesForExtract(*target.extendDecl, sel, inheritedTypes);
        return;
    }
    if (target.inheritableDecl) {
        CollectInheritedTypesForExtract(*target.inheritableDecl, sel, inheritedTypes);
    }
}

void CollectSelectedEditsFromTarget(const TargetDecl &target, SelectedEditsRequest &request)
{
    switch (target.kind) {
        case TargetKind::CLASS:
            CollectSelectedEditsFromType(*target.classDecl, request);
            return;
        case TargetKind::STRUCT:
            CollectSelectedEditsFromType(*target.structDecl, request);
            return;
        case TargetKind::INTERFACE:
            CollectSelectedEditsFromType(*target.interfaceDecl, request, true);
            return;
        case TargetKind::ENUM:
            CollectSelectedEditsFromType(*target.enumDecl, request);
            return;
        case TargetKind::EXTEND:
            request.addOverride = false;
            CollectSelectedEditsFromType(*target.extendDecl, request, false, false);
            return;
        default:
            return;
    }
}

std::unordered_map<std::string, const InterfaceInfo::MemberMeta *> BuildMemberMetaMap(const InterfaceInfo &info)
{
    std::unordered_map<std::string, const InterfaceInfo::MemberMeta *> metaMap;
    metaMap.reserve(info.metas.size());
    for (const auto &meta : info.metas) {
        metaMap.emplace(meta.signature, &meta);
    }
    return metaMap;
}

std::string BuildInterfaceHeaderText(const InterfaceInfo &info, const std::string &packageName)
{
    std::string text;
    if (!packageName.empty()) {
        text += "package " + packageName + "\n\n";
    }
    text += "public interface " + info.name + info.genericParams;
    if (!info.inheritedTypes.empty()) {
        text += " <: ";
        text += JoinTypes(info.inheritedTypes);
    }
    text += info.genericWhereClause;
    text += " {\n";
    return text;
}

std::string BuildInterfaceMemberHeader(const std::string &member, const InterfaceInfo::MemberMeta *meta)
{
    std::string header = INTERFACE_MEMBER_INDENT;
    if (meta && meta->isStatic) {
        header += "static ";
    }
    if (meta && meta->isMut) {
        header += "mut ";
    }
    if (meta && meta->isOperator) {
        header += "operator ";
    }
    header += "func " + member;
    return header;
}
// LCOV_EXCL_BR_START
void AppendInterfaceMemberText(std::string &text,
                               const InterfaceInfo &info,
                               const std::string &member,
                               const InterfaceInfo::MemberMeta *meta,
                               bool withMethodBodies)
{
    auto commentIt = info.memberComments.find(member);
    if (commentIt != info.memberComments.end() && !commentIt->second.empty()) {
        text += TweakUtils::IndentTextBlock(commentIt->second, INTERFACE_MEMBER_INDENT);
        text += "\n";
    }

    std::string header = BuildInterfaceMemberHeader(member, meta);
    auto bodyIt = info.implBodies.find(member);
    if (withMethodBodies && bodyIt != info.implBodies.end() && !bodyIt->second.empty()) {
        text += header + " " + bodyIt->second + "\n";
    } else {
        text += header + "\n";
    }
}

bool ExtractInterface::Prepare(const Tweak::Selection &sel)
{
    TweakRuleEngine ruleEngine;
    ruleEngine.AddRule(std::make_unique<ExtractInterfaceSelectionRule>());
    if (!ruleEngine.CheckRules(sel, extraOptions)) {
        return true;
    }

    auto target = ResolveTargetDecl(sel);
    if (!target.has_value()) {
        return true;
    }

    InterfaceInfo info;
    extraOptions["suggestName"] = target->name;
    CollectMembersFromTarget(*target, sel, info);

    std::vector<std::string> inheritedTypeMembers;
    CollectInheritedTypesFromTarget(*target, sel, inheritedTypeMembers);

    std::string membersJson = "[";
    bool first = true;
    for (const auto &typeName : inheritedTypeMembers) {
        AppendMemberJson(membersJson, first, std::string(INHERITED_MEMBER_PREFIX) + " " + typeName, false, "public");
    }
    for (const auto &meta : info.metas) {
        AppendMemberJson(membersJson, first, meta.signature, meta.isStatic, meta.visibility);
    }
    membersJson.push_back(']');
    extraOptions["members"] = std::move(membersJson);
    return true;
}
// LCOV_EXCL_BR_STOP
std::string BuildInterfaceDeclText(const InterfaceInfo &info,
                                   const std::string &packageName,
                                   bool withMethodBodies)
{
    const size_t doubleNewLineLength = std::string("\n\n").size();
    std::string text = BuildInterfaceHeaderText(info, packageName);
    auto metaMap = BuildMemberMetaMap(info);

    bool firstMember = true;
    for (const auto &member : info.members) {
        if (!firstMember &&
            (text.size() < doubleNewLineLength ||
             text.substr(text.size() - doubleNewLineLength) != "\n\n")) {
            text += "\n";
        }
        firstMember = false;
        const InterfaceInfo::MemberMeta *meta = nullptr;
        auto it = metaMap.find(member);
        if (it != metaMap.end()) {
            meta = it->second;
        }
        AppendInterfaceMemberText(text, info, member, meta, withMethodBodies);
    }
    text += "}\n";
    return text;
}

TextEdit InsertInterfaceDecl(const Tweak::Selection &sel, const InterfaceInfo &info)
{
    TextEdit edit;
    if (!sel.arkAst || !sel.arkAst->file) {
        return edit;
    }

    Range insertRange = TweakUtils::FindGlobalInsertPos(*sel.arkAst->file, sel.range);
    insertRange = TransformFromChar2IDE(insertRange);
    edit.range = insertRange;
    edit.newText = "\n\n" + BuildInterfaceDeclText(info, "", true);
    return edit;
}

struct ClassInheritUpdate {
    bool hasRealInherited = false;
    Cangjie::Position first = Cangjie::INVALID_POSITION;
    Cangjie::Position last = Cangjie::INVALID_POSITION;
    std::vector<std::string> keptTypes;
    std::unordered_set<std::string> keptNormalized;
};
// LCOV_EXCL_BR_START
ClassInheritUpdate CollectClassInheritUpdate(Cangjie::AST::InheritableDecl &decl,
                                             SourceManager *sm,
                                             const std::unordered_set<std::string> &absorbedInterfaces)
{
    ClassInheritUpdate update;
    for (auto &ty : decl.inheritedTypes) {
        if (!ty || (ty->GetTy() && ty->GetTy()->IsObject())) {
            continue;
        }
        update.hasRealInherited = true;
        if (update.first.IsZero()) {
            update.first = ty->GetBegin();
        }
        update.last = ty->GetEnd();

        if (IsInterfaceInheritedType(ty)) {
            std::string name = ResolveInheritedTypeText(ty, sm);
            if (!name.empty() && absorbedInterfaces.count(NormalizeTypeNameForCompare(name)) != 0) {
                continue;
            }
        }
        std::string name = ResolveInheritedTypeText(ty, sm);
        if (name.empty()) {
            continue;
        }
        std::string normalized = NormalizeTypeNameForCompare(name);
        if (update.keptNormalized.insert(normalized).second) {
            update.keptTypes.push_back(std::move(name));
        }
    }
    return update;
}

std::unordered_set<std::string> BuildAbsorbedInterfaceSet(const InterfaceInfo &info)
{
    std::unordered_set<std::string> absorbedInterfaces;
    for (const auto &inheritedType : info.inheritedTypes) {
        std::string normalized = NormalizeTypeNameForCompare(inheritedType);
        if (!normalized.empty()) {
            absorbedInterfaces.insert(std::move(normalized));
        }
    }
    return absorbedInterfaces;
}
// LCOV_EXCL_BR_STOP
void AddInterfaceTypeIfNeeded(ClassInheritUpdate &update, const std::string &interfaceTypeName)
{
    std::string newNameNorm = NormalizeTypeNameForCompare(interfaceTypeName);
    if (!newNameNorm.empty() && update.keptNormalized.insert(newNameNorm).second) {
        update.keptTypes.push_back(interfaceTypeName);
    }
    if (update.keptTypes.empty()) {
        update.keptTypes.push_back(interfaceTypeName);
    }
}

TextEdit BuildClassImplementsClauseEdit(Cangjie::AST::InheritableDecl &decl,
                                        SourceManager *sm,
                                        const InterfaceInfo &info)
{
    TextEdit edit;
    auto absorbedInterfaces = BuildAbsorbedInterfaceSet(info);
    auto update = CollectClassInheritUpdate(decl, sm, absorbedInterfaces);
    std::string interfaceTypeName = info.name + info.genericParams;
    AddInterfaceTypeIfNeeded(update, interfaceTypeName);

    if (!update.hasRealInherited || update.first.IsZero() || update.last.IsZero()) {
        return BuildInsertAtDeclTail(decl, sm, " <: " + interfaceTypeName);
    }

    std::string replacedText = JoinTypes(update.keptTypes);
    Range where = {update.first, update.last};
    where = TransformFromChar2IDE(where);
    edit.range = where;
    edit.newText = std::move(replacedText);
    return edit;
}
// LCOV_EXCL_BR_STOP
// LCOV_EXCL_START
bool IsSameTypeName(const std::string &lhs, const std::string &rhs)
{
    std::string lhsTrim = Trim(lhs);
    std::string rhsTrim = Trim(rhs);
    if (lhsTrim == rhsTrim) {
        return true;
    }
    return NormalizeTypeNameForCompare(lhsTrim) == NormalizeTypeNameForCompare(rhsTrim);
}

bool AlreadyInheritsInterface(Cangjie::AST::InheritableDecl &decl,
                              SourceManager *sm,
                              const std::string &interfaceTypeName)
{
    for (auto &ty : decl.inheritedTypes) {
        if (!ty || (ty->GetTy() && ty->GetTy()->IsObject())) {
            continue;
        }
        if (IsSameTypeName(ResolveInheritedTypeText(ty, sm), interfaceTypeName)) {
            return true;
        }
    }
    return false;
}

Cangjie::Position FindLastRealInheritedTypeEnd(Cangjie::AST::InheritableDecl &decl)
{
    Cangjie::Position tail = Cangjie::INVALID_POSITION;
    for (auto &ty : decl.inheritedTypes) {
        if (!ty || (ty->GetTy() && ty->GetTy()->IsObject())) {
            continue;
        }
        tail = ty->GetEnd();
    }
    return tail;
}

TextEdit BuildGeneralImplementsClauseEdit(Cangjie::AST::InheritableDecl &decl,
                                          SourceManager *sm,
                                          const InterfaceInfo &info)
{
    TextEdit edit;
    std::string interfaceTypeName = info.name + info.genericParams;
    if (AlreadyInheritsInterface(decl, sm, interfaceTypeName)) {
        return edit;
    }

    Cangjie::Position tail = FindLastRealInheritedTypeEnd(decl);
    if (!HasRealInheritedType(decl) || tail.IsZero()) {
        return BuildInsertAtDeclTail(decl, sm, " <: " + interfaceTypeName);
    }

    Range where = {tail, tail};
    where = TransformFromChar2IDE(where);
    edit.range = where;
    edit.newText = " & " + interfaceTypeName;
    return edit;
}
// LCOV_EXCL_STOP
TextEdit InsertImplementsClause(Cangjie::AST::InheritableDecl &decl,
                                const Tweak::Selection &sel,
                                const InterfaceInfo &info)
{
    if (!sel.arkAst) {
        return TextEdit{};
    }
    auto *sm = sel.arkAst->sourceManager;

    return BuildClassImplementsClauseEdit(decl, sm, info);
}

TextEdit InsertImplementationInterfaceClause(Cangjie::AST::InheritableDecl &decl,
                                             const Tweak::Selection &sel,
                                             const InterfaceInfo &info)
{
    if (!sel.arkAst) {
        return TextEdit{};
    }
    return BuildGeneralImplementsClauseEdit(decl, sel.arkAst->sourceManager, info);
}

TextEdit InsertImplementsClause(Cangjie::AST::ExtendDecl &decl,
                                const Tweak::Selection &sel,
                                const InterfaceInfo &info)
{
    if (!sel.arkAst || !sel.arkAst->sourceManager) {
        return TextEdit{};
    }
    auto *sm = sel.arkAst->sourceManager;

    std::string interfaceTypeName = info.name + info.genericParams;
    auto update = CollectClassInheritUpdate(decl, sm, BuildAbsorbedInterfaceSet(info));
    AddInterfaceTypeIfNeeded(update, interfaceTypeName);

    if (!update.hasRealInherited || update.first.IsZero() || update.last.IsZero()) {
        return BuildInsertAtDeclTail(decl, sm, " <: " + interfaceTypeName);
    }

    Range where = {update.first, update.last};
    where = TransformFromChar2IDE(where);
    TextEdit edit;
    edit.range = where;
    edit.newText = JoinTypes(update.keptTypes);
    return edit;
}

TextEdit InsertInterfaceDeclToTargetFile(const std::string &targetPath, const InterfaceInfo &info)
{
    TextEdit edit;
    Range insertRange;
    insertRange.start = {0, 0, 0};
    insertRange.end = insertRange.start;

    edit.range = insertRange;
    edit.newText = BuildInterfaceDeclText(info, ResolveTargetPackageName(targetPath), true);
    return edit;
}

void AppendCreateFileDocumentChange(Tweak::Effect &effect, const std::string &targetPath, const TextEdit &edit)
{
    if (targetPath.empty()) {
        return;
    }

    std::string targetUri = URI::URIFromAbsolutePath(targetPath).ToString();

    CreateFile createFile;
    createFile.uri = targetUri;
    nlohmann::json createFileJson;
    ToJSON(createFile, createFileJson);
    effect.documentChanges.push_back(std::move(createFileJson));

    TextDocumentEdit textDocumentEdit;
    textDocumentEdit.textDocument.uri.file = targetUri;
    textDocumentEdit.textDocument.version = -1;
    textDocumentEdit.textEdits.push_back(edit);
    nlohmann::json textEditJson;
    ToJSON(textDocumentEdit, textEditJson);
    effect.documentChanges.push_back(std::move(textEditJson));
}

void InitializeApplyContext(ApplyContext &context)
{
    context.info.genericParams = ResolveTargetGenericParams(context.target);
    context.info.genericWhereClause =
        ResolveTargetGenericWhereClause(context.target, context.sel.arkAst->sourceManager);
    context.renameOriginalClass = (context.target.kind == TargetKind::CLASS) &&
                                  ParseBoolOption(context.sel.extraOptions,
                                      "renameOriginalClassAndUseInterfaceWherePossible", false);
    context.targetPath = NormalizeTargetPath(context.sel.extraOptions);
    std::string requestedName;
    auto itName = context.sel.extraOptions.find("interfaceName");
    if (itName != context.sel.extraOptions.end()) {
        requestedName = Trim(itName->second);
        context.hasExplicitInterfaceName = !requestedName.empty();
    } else if ((itName = context.sel.extraOptions.find("suggestName")) != context.sel.extraOptions.end()) {
        requestedName = Trim(itName->second);
    }

    if (context.renameOriginalClass) {
        context.info.name = context.target.name;
        context.implementationClassName = ResolveImplementationClassName(context.sel.extraOptions, "");
        if (context.implementationClassName.empty()) {
            context.implementationClassName = context.target.name + "Impl";
        }
        context.hasExplicitInterfaceName = true;
    } else {
        context.info.name = requestedName.empty() ? context.target.name : requestedName;
    }

    context.targetPath = ResolveTargetFilePath(context.targetPath, context.info.name);
    // LCOV_EXCL_START
    if (!context.hasExplicitInterfaceName && !context.targetPath.empty() &&
        Cangjie::FileUtil::GetFileExtension(context.targetPath) == "cj") {
        std::string targetFileName = Cangjie::FileUtil::GetFileNameWithoutExtension(context.targetPath);
        if (!targetFileName.empty()) {
            context.info.name = targetFileName;
        }
    }
    // LCOV_EXCL_STOP
    context.chosen = ParseSelectedMembers(context.sel.extraOptions);
    context.hasSelectedMembersOption = context.sel.extraOptions.find("selectedMembers") != context.sel.extraOptions.end();
    context.selectedInheritedTypes = ParseSelectedInheritedMemberTypes(context.chosen);
    context.withImplementation = ParseBoolOption(context.sel.extraOptions, "withImplementation", true);
    if (context.renameOriginalClass) {
        context.withImplementation = true;
    }
}

void CollectApplyInterfaceInfo(ApplyContext &context)
{
    CollectMembersFromTarget(context.target, context.sel, context.info);

    if (context.renameOriginalClass && context.target.classDecl) {
        CollectClassInterfaceInheritedTypesForRename(
            *context.target.classDecl, context.sel, context.info.inheritedTypes);
    } else if (context.target.classDecl) {
        CollectInterfaceInheritedTypesForExtract(
            *context.target.classDecl, context.sel, context.selectedInheritedTypes, context.info.inheritedTypes);
    } else if (context.target.interfaceDecl) {
        CollectInterfaceInheritedTypesForExtract(
            *context.target.interfaceDecl, context.sel, context.selectedInheritedTypes, context.info.inheritedTypes);
    } else if (context.target.kind != TargetKind::CLASS) {
        std::vector<std::string> inheritedTypes;
        CollectInheritedTypesFromTarget(context.target, context.sel, inheritedTypes);
        for (auto &typeName : inheritedTypes) {
            if (context.selectedInheritedTypes.count(NormalizeTypeNameForCompare(typeName))) {
                context.info.inheritedTypes.push_back(std::move(typeName));
            }
        }
    }
}

void FilterMembersBySelection(InterfaceInfo &info,
                              const std::unordered_set<std::string> &chosen,
                              bool hasSelectedMembersOption)
{
    if (!hasSelectedMembersOption) {
        return;
    }
    std::vector<std::string> filteredMembers;
    filteredMembers.reserve(info.members.size());
    for (auto &member : info.members) {
        if (chosen.count(member) || chosen.count(TweakUtils::NormalizeSignature(member))) {
            filteredMembers.push_back(member);
        }
    }
    info.members.swap(filteredMembers);
}

void FilterSelectedMembers(ApplyContext &context)
{
    FilterMembersBySelection(context.info, context.chosen, context.hasSelectedMembersOption);
}

void AddRenameOriginalClassEdit(ApplyContext &context)
{
    if (!context.renameOriginalClass || !context.target.classDecl) {
        return;
    }

    auto nameBegin = context.target.classDecl->identifier.Begin();
    if (nameBegin.IsZero() || context.implementationClassName.empty()) {
        return;
    }
    auto nameEnd = context.target.classDecl->identifier.End();
    Range renameRange{nameBegin, nameEnd};
    renameRange = TransformFromChar2IDE(renameRange);
    context.effect.applyEdits[context.sourceUri].push_back(TextEdit{renameRange, context.implementationClassName});
}

void AddInterfaceDeclarationEdits(ApplyContext &context)
{
    if (context.targetPath.empty()) {
        context.effect.applyEdits.emplace(
            context.sourceUri, std::vector<TextEdit>{InsertInterfaceDecl(context.sel, context.info)});
        return;
    }
    AppendCreateFileDocumentChange(
        context.effect,
        context.targetPath,
        InsertInterfaceDeclToTargetFile(context.targetPath, context.info));
}

void AddImplementationImportsAndInheritance(ApplyContext &context)
{
    auto importEdit = BuildImportInterfaceEdit(context.sel, context.targetPath, context.info.name);
    if (importEdit.has_value()) {
        context.effect.applyEdits[context.sourceUri].push_back(std::move(*importEdit));
    }

    TextEdit targetImplementsEdit;
    if (context.target.extendDecl) {
        targetImplementsEdit = InsertImplementsClause(*context.target.extendDecl, context.sel, context.info);
    } else if (context.target.inheritableDecl) {
        targetImplementsEdit = context.renameOriginalClass
            ? InsertImplementationInterfaceClause(*context.target.inheritableDecl, context.sel, context.info)
            : InsertImplementsClause(*context.target.inheritableDecl, context.sel, context.info);
    }
    if (!targetImplementsEdit.newText.empty()) {
        context.effect.applyEdits[context.sourceUri].push_back(std::move(targetImplementsEdit));
    }
}

void AddSelectedMemberEdits(ApplyContext &context)
{
    if (context.chosen.empty()) {
        return;
    }

    SelectedEditsRequest request{
        context.sel,
        context.chosen,
        context.effect.applyEdits[context.sourceUri],
        context.target.kind != TargetKind::EXTEND
    };
    CollectSelectedEditsFromTarget(context.target, request);
}

Cangjie::AST::Decl *ResolveTypeReplacementTargetDecl(const TargetDecl &target)
{
    switch (target.kind) {
        case TargetKind::CLASS:
            return target.classDecl;
        case TargetKind::STRUCT:
            return target.structDecl;
        case TargetKind::INTERFACE:
            return target.interfaceDecl;
        case TargetKind::ENUM:
            return target.enumDecl;
        case TargetKind::EXTEND:
            if (!target.extendDecl || !target.extendDecl->extendedType) {
                return nullptr;
            }
            return target.extendDecl->extendedType->GetTarget();
        default:
            return nullptr;
    }
}

void AddTypeReferenceReplacementEdits(ApplyContext &context)
{
    auto *replacementTarget = ResolveTypeReplacementTargetDecl(context.target);
    if (!replacementTarget || context.info.name.empty()) {
        return;
    }

    TypeReplacementContext replacementContext{
        context.sel,
        *replacementTarget,
        context.info.name,
        context.renameOriginalClass ? context.implementationClassName : "",
        context.targetPath.empty() ? context.sel.arkAst->file->filePath : context.targetPath,
        BuildAllowedMemberNames(context.info.members),
        context.effect.applyEdits
    };
    CollectTypeReferenceReplacementEdits(replacementContext);
}

std::optional<Tweak::Effect> ExtractInterface::Apply(const Tweak::Selection &sel)
{
    if (!sel.arkAst || !sel.arkAst->sourceManager || !sel.arkAst->file) {
        return std::nullopt;
    }

    auto target = ResolveTargetDecl(sel);
    if (!target.has_value()) {
        return std::nullopt;
    }

    ApplyContext context{
        sel,
        *target,
        Tweak::Effect{},
        InterfaceInfo{},
        "",
        "",
        "",
        {},
        {},
        false,
        false,
        false,
        true
    };
    context.sourceUri = URI::URIFromAbsolutePath(sel.arkAst->file->filePath).ToString();

    InitializeApplyContext(context);
    CollectApplyInterfaceInfo(context);
    FilterSelectedMembers(context);

    AddInterfaceDeclarationEdits(context);
    if (!context.withImplementation) {
        return context.effect;
    }

    AddImplementationImportsAndInheritance(context);
    AddRenameOriginalClassEdit(context);
    AddSelectedMemberEdits(context);
    AddTypeReferenceReplacementEdits(context);
    return context.effect;
}

namespace {

// LCOV_EXCL_BR_START
std::set<Location> ParseSelectedTypeReferences(const std::map<std::string, std::string> &options)
{
    std::set<Location> selectedReferences;
    auto it = options.find("selectedTypeReferences");
    if (it == options.end()) {
        return selectedReferences;
    }

    const std::string &raw = it->second;
    if (raw.empty() || raw.front() != '[') {
        return selectedReferences;
    }

    nlohmann::json json = nlohmann::json::parse(raw, nullptr, false);
    if (json.is_discarded() || !json.is_array()) {
        return selectedReferences;
    }

    for (const auto &entry : json) {
        auto location = ParseSelectedTypeReferenceEntry(entry);
        if (!location.has_value() || location->uri.file.empty()) {
            continue;
        }
        selectedReferences.insert(std::move(*location));
    }
    return selectedReferences;
}

void CollectIndexedTypeReferences(TypeReplacementContext &context, ReferencesResult &result)
{
    FindReferencesImpl::CompileDownStreamPackage({&context.targetDecl});
    auto index = CompilerCangjieProject::GetInstance()->GetIndex();
    auto id = GetSymbolId(context.targetDecl);
    if (!index || id == lsp::INVALID_SYMBOL_ID) {
        return;
    }
    std::unordered_set<lsp::SymbolID> ids;
    lsp::SymbolID topId = id;
    index->FindRiddenUp(id, ids, topId);
    index->FindRiddenDown(topId, ids);
    (void)ids.insert(id);

    lsp::RefsRequest req{ids, lsp::RefKind::REFERENCE, std::nullopt};
    lsp::Ref definition{{}, {}, 0};
    index->RefsFindReference(req, definition, [&result](const lsp::Ref &ref) {
        if (ref.isCjoRef || ref.location.IsZeroLoc()) {
            return;
        }
        auto realPath = ref.location.fileUri;
        if (EndsWith(realPath, ".macrocall")) {
            return;
        }
        CompilerCangjieProject::GetInstance()->GetRealPath(realPath);
        Location loc{{URI::URIFromAbsolutePath(realPath).ToString()},
                     TransformFromChar2IDE({ref.location.begin, ref.location.end})};
        (void)result.References.emplace(loc);
    });
}

ReferencesResult CollectTypeReferences(TypeReplacementContext &context)
{
    ReferencesResult result;
    if (IsGlobalOrMemberOrItsParam(context.targetDecl)) {
        CollectIndexedTypeReferences(context, result);
    } else if (context.sel.arkAst) {
        FindReferencesImpl::GetCurPkgUesage(&context.targetDecl, *context.sel.arkAst, result);
    }
    return result;
}

void AddTypeReplacementEditForReference(TypeReplacementContext &context, const Location &location)
{
    std::optional<TextEdit> importEdit;
    auto edit = BuildTypeReplacementEditForLocation(location, context, importEdit);
    auto &edits = context.applyEdits[location.uri.file];
    if (edit.has_value() && importEdit.has_value()) {
        if (std::find(edits.begin(), edits.end(), *importEdit) == edits.end()) {
            edits.push_back(std::move(*importEdit));
        }
    }
    if (edit.has_value() && std::find(edits.begin(), edits.end(), *edit) == edits.end()) {
        edits.push_back(std::move(*edit));
    }

    auto implementationEdit = BuildImplementationClassReplacementEditForLocation(location, context);
    if (implementationEdit.has_value() &&
        std::find(edits.begin(), edits.end(), *implementationEdit) == edits.end()) {
        edits.push_back(std::move(*implementationEdit));
    }
}

} // namespace

void CollectTypeReferenceReplacementEdits(TypeReplacementContext &context)
{
    if (!context.sel.arkAst || !context.sel.arkAst->file || context.interfaceName.empty()) {
        return;
    }

    ReferencesResult result = CollectTypeReferences(context);
    bool hasSelectedReferencesFilter = context.sel.extraOptions.find("hasSelectedTypeReferences") !=
        context.sel.extraOptions.end() && ParseBoolOption(context.sel.extraOptions, "hasSelectedTypeReferences", false);
    std::set<Location> selectedReferences = ParseSelectedTypeReferences(context.sel.extraOptions);

    for (const auto &location : result.References) {
        Location normalizedLocation = location;
        normalizedLocation.uri.file = NormalizeUriString(location.uri.file);
        if (hasSelectedReferencesFilter &&
            !std::any_of(selectedReferences.begin(), selectedReferences.end(),
                         [&normalizedLocation](const Location &selected) {
                             return IsSameSelectedTypeReference(selected, normalizedLocation);
                         })) {
            continue;
        }
        AddTypeReplacementEditForReference(context, normalizedLocation);
    }
}
// LCOV_EXCL_BR_STOP

} // namespace ark
