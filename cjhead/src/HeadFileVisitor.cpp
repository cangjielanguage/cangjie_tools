// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "HeadFileVisitor.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>


#include "cangjie/AST/Comment.h"
#include "cangjie/AST/PrintNode.h"
#include "cangjie/AST/Node.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/AST/Match.h"
#include "cangjie/Basic/Match.h"

using namespace Cangjie;
using namespace Cangjie::AST;

namespace CJHead {
std::unordered_map<std::string, std::string> HeadFileVisitor::typealias_to_content_;

std::unordered_map<std::string, int> HeadFileVisitor::typealias_to_line_;

std::unordered_map<std::string, bool> HeadFileVisitor::typealias_to_appear_;

std::unordered_map<std::string, std::vector<std::string>> HeadFileVisitor::typealias_to_comment_;

std::unordered_set<std::string> function_map;

std::unordered_set<std::string> removed_extend;

std::unordered_set<std::string> abstract_class;

HeadFileVisitor::HeadFileVisitor(
    const std::unique_ptr<CJHeadCompilerInstance> &cjhead_compiler_instance, const OwnedPtr<File> &cur_file)
    : cjhead_compiler_instance_(cjhead_compiler_instance), cur_file_(cur_file),
      source_manager_(cjhead_compiler_instance->GetSourceManager())
{
    InitVisitFuncMap();
}

void HeadFileVisitor::InitVisitFuncMap()
{
    visit_func_map_ = {
        {ASTKind::STRUCT_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitStructDecl(d, dep, f); }},
        {ASTKind::FUNC_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitFuncDecl(d, dep, f); }},
        {ASTKind::VAR_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitVarDecl(d, dep, f); }},
        {ASTKind::CLASS_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitClassDecl(d, dep, f); }},
        {ASTKind::INTERFACE_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitInterfaceDecl(d, dep, f); }},
        {ASTKind::ENUM_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitEnumDecl(d, dep, f); }},
        {ASTKind::EXTEND_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitExtendDecl(d, dep, f); }},
        {ASTKind::PROP_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitPropDecl(d, dep, f); }},
        {ASTKind::PRIMARY_CTOR_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitPrimaryCtorDecl(d, dep, f); }},
        {ASTKind::MACRO_EXPAND_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitMacroExpandDecl(d, dep, f); }},
        {ASTKind::MACRO_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitMacroDecl(d, dep, f); }},
        {ASTKind::TYPE_ALIAS_DECL,
            [this](const OwnedPtr<Decl> &d, const int &dep, const bool &f) { VisitTypealiasDecl(d, dep, f); }},
    };
}

void HeadFileVisitor::Run()
{
    auto source_packages = cjhead_compiler_instance_->GetSourcePackages();
    std::vector<Token> comments = CJHeadUtil::GetComments(cur_file_->filePath);
    VisitFirstComment(comments);
    VisitPackage(source_packages);
    std::vector<OwnedPtr<Decl>> &decls = cur_file_->decls;
    decls.erase(std::remove_if(decls.begin(),
        decls.end(),
        [this](auto &x) {
            if (!IsPublic(x)) {
                function_map.insert(x->identifier.Val());
                return true;
            }
            return false;
        }),

        decls.end());

    for (const auto &decl : decls) {
        VisitNode(decl);
    }

    decls.erase(std::remove_if(decls.begin(),
        decls.end(),
        [this](auto &x) {
            if (x->astKind == ASTKind::EXTEND_DECL) {
                auto extend_decl = dynamic_cast<ExtendDecl *>(x.get().get());
                return removed_extend.count(extend_decl->extendedType->ToString()) != 0;
            }
            return false;
        }),
        decls.end());
}

auto HeadFileVisitor::VisitFirstComment(const std::vector<Token> &comments) -> bool
{
    if (!comments.empty()) {
        if (comments.front().Begin().line == 1 && comments.front().Begin().column == 1) {
            first_comment_ = comments.front().Value() + "\n";
        }
    }
    return true;
}

bool IsAbstractOrOpen(const OwnedPtr<Decl> &decl)
{
    for (auto &modifier : decl->modifiers) {
        if (modifier.ToString() == "sealed") {
            return false;
        }
    }
    for (auto &modifier : decl->modifiers) {
        if (modifier.ToString() == "open") {
            return true;
        }
        if (modifier.ToString() == "abstract") {
            return true;
        }
    }
    return false;
}

auto HeadFileVisitor::VisitPackage(std::vector<Ptr<Package>> &source_package) -> bool
{
    Ptr<Package> package = source_package.front();
    std::string package_info = (package->isMacroPackage ? "macro " : "");
    package_info += "package " + package->fullPackageName + "\n";
    import_info_.emplace_back(package_info);

    std::vector<OwnedPtr<ImportSpec>> &imports = (cur_file_)->imports;
    for (size_t i = 0; i < imports.size(); ++i) {
        auto &import = imports[i];
        auto comment_strings = GetCommentString(import);
        if (!comment_strings.empty()) {
            import_info_.emplace_back("\n");
            for (const auto &line : comment_strings) {
                import_info_.emplace_back(line + "\n");
            }
        }
        struct ImportContent &import_content = import->content;
        auto import_content_string = source_manager_.GetContentBetween(import->begin, import_content.end);
        if (!import_content.rightCurlPos.IsZero() && import_content_string.back() != '}') {
            import_content_string += "}";
        }
        import_info_.emplace_back(import_content_string + "\n");
        if (!(import_content.items.empty())) {
            i += import_content.items.size();
        }
    }
    return true;
}

Ptr<const Node> GetCurMacro(const Decl &decl)
{
    Ptr<const Node> curCall = nullptr;
    if (decl.curMacroCall) {
        curCall = decl.curMacroCall;
    } else {
        Ptr<const Decl> tmpDecl = &decl;
        while (tmpDecl->outerDecl) {
            tmpDecl = tmpDecl->outerDecl;
            if (tmpDecl->curMacroCall) {
                curCall = tmpDecl->curMacroCall;
                break;
            }
        }
    }
    return curCall;
}

std::vector<CommentGroup> FindCommentsInMacroCall(const Ptr<const Decl> node, Ptr<const Node> curCall)
{
    std::vector<CommentGroup> comments;
    if (!node || !curCall) {
        return comments;
    }
    bool ret = false;
    std::string id = node->identifier;
    auto begin = node->GetBegin();
    auto end = node->GetEnd();

    if (!curCall) {
        return comments;
    }

    auto addComments = [&comments](const Decl &decl) {
        auto lead_comments_group = decl.comments.leadingComments;
        for (auto &lead_comment : lead_comments_group) {
            comments.push_back(lead_comment);
        }
    };

    ConstWalker walker(
        curCall, [&ret, &id, &begin, &end, &comments, &addComments](Ptr<const Node> node) -> VisitAction {
            Meta::match (*node)([&ret, &id, &begin, &end, &comments, &addComments](const Decl &decl) {
                if (decl.identifier == id && begin == decl.GetBegin() && end == decl.GetEnd() &&
                    !decl.comments.IsEmpty()) {
                    addComments(decl);
                    ret = true;
                }
            });
            if (ret) {
                return VisitAction::STOP_NOW;
            }
            return VisitAction::WALK_CHILDREN;
        });
    walker.Walk();
    return comments;
}

void HeadFileVisitor::VisitNode(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    if (depth == 0 && !IsPublic(decl) && !from_interface && decl->astKind != ASTKind::TYPE_ALIAS_DECL &&
        decl->astKind != ASTKind::EXTEND_DECL) {
        return;
    }

    auto macroCall = GetCurMacro(*decl.get().get());
    auto comments = FindCommentsInMacroCall(decl, macroCall);
    for (auto &comment : comments) {
        decl->comments.leadingComments.push_back(comment);
    }

    std::string src_identifier = GetSrcIndentifierString(decl);
    auto visit_func = visit_func_map_.find(decl->astKind);
    if (visit_func != visit_func_map_.end()) {
        visit_func->second(decl, depth, from_interface);
    }
}

/* -- start of visit */
void HeadFileVisitor::VisitStructDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto struct_decl = dynamic_cast<StructDecl *>(decl.get().get());
    auto &member_decls = struct_decl->GetMemberDecls();

    auto &inherited_types = struct_decl->inheritedTypes;
    inherited_types.erase(
        std::remove_if(
            inherited_types.begin(), inherited_types.end(), [this](auto &x) { return x->ToString() == "Object"; }),
        inherited_types.end());

    member_decls.erase(
        std::remove_if(member_decls.begin(), member_decls.end(), [this](auto &x) { return !IsPublic(x); }),
        member_decls.end());

    for (const auto &member_decl : member_decls) {
        if (IsPublic(member_decl)) {
            if (!member_decl->TestAttr(Attribute::COMPILER_ADD)) {
                VisitNode(member_decl, depth + 1);
            }
        }
    }

    if (IsAbstractOrOpen(decl)) {
        abstract_class.insert(decl->identifier.Val());
    }

    if (!IsPublic(decl)) {
        function_map.insert(decl->identifier.Val());
    }
}

void GetPrimaryCtorAnnotations(const OwnedPtr<Decl> &decl)
{
    auto class_decl = dynamic_cast<ClassDecl *>(decl.get().get());
    auto &member_decls = class_decl->GetMemberDecls();
    for (const auto &member_decl : member_decls) {
        if (member_decl->astKind != ASTKind::PRIMARY_CTOR_DECL) {
            continue;
        }
        for (const auto &func_decl : member_decls) {
            if (!(func_decl->astKind == ASTKind::FUNC_DECL && func_decl->identifier.Val() == "init" &&
                func_decl->TestAttr(Attribute::COMPILER_ADD))) {
                    continue;
                }
                auto init_decl = dynamic_cast<FuncDecl *>(func_decl.get().get());
                auto &annotations = init_decl->annotations;
                for (auto& anno : annotations) {
                    member_decl->annotations.emplace_back(std::move(anno));
                }
        }
    }
}

void HeadFileVisitor::VisitClassDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto class_decl = dynamic_cast<ClassDecl *>(decl.get().get());
    auto &member_decls = class_decl->GetMemberDecls();
    GetPrimaryCtorAnnotations(decl);
    auto &inherited_types = class_decl->inheritedTypes;
    inherited_types.erase(
        std::remove_if(
            inherited_types.begin(), inherited_types.end(), [this](auto &x) { return x->ToString() == "Object"; }),
        inherited_types.end());

    member_decls.erase(
        std::remove_if(member_decls.begin(),
            member_decls.end(),
            [&decl, this](auto &x) { return !(IsPublic(x) || (IsAbstractOrOpen(decl) && IsProtected(x))); }),
        member_decls.end());

    member_decls.erase(
        std::remove_if(member_decls.begin(),
            member_decls.end(),
            [&decl, this](auto &x) { return x->TestAttr(Attribute::COMPILER_ADD); }),
        member_decls.end());

    for (const auto &member_decl : member_decls) {
        if (IsPublic(member_decl) || (IsAbstractOrOpen(decl) && IsProtected(member_decl))) {
            if (!member_decl->TestAttr(Attribute::COMPILER_ADD)) {
                VisitNode(member_decl, depth + 1);
            }
        }
    }

    if (IsAbstractOrOpen(decl)) {
        abstract_class.insert(decl->identifier.Val());
    }

    if (!IsPublic(decl)) {
        function_map.insert(decl->identifier.Val());
    }
}

void HeadFileVisitor::VisitInterfaceDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto interface_decl = dynamic_cast<InterfaceDecl *>(decl.get().get());
    auto &member_decls = interface_decl->GetMemberDecls();
    auto &inherited_types = interface_decl->inheritedTypes;

    inherited_types.erase(
        std::remove_if(
            inherited_types.begin(), inherited_types.end(), [this](auto &x) { return x->ToString() == "Object"; }),
        inherited_types.end());

    for (const auto &member_decl : member_decls) {
        VisitNode(member_decl, depth + 1, true);
    }
}

void HeadFileVisitor::VisitEnumDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto enum_decl = dynamic_cast<EnumDecl *>(decl.get().get());
    VisitEnumConstructor(enum_decl);
    auto &member_decls = enum_decl->GetMemberDecls();
    auto &inherited_types = enum_decl->inheritedTypes;

    inherited_types.erase(
        std::remove_if(
            inherited_types.begin(), inherited_types.end(), [this](auto &x) { return x->ToString() == "Object"; }),
        inherited_types.end());

    member_decls.erase(
        std::remove_if(member_decls.begin(), member_decls.end(), [this](auto &x) { return !IsPublic(x); }),
        member_decls.end());

    for (const auto &member_decl : member_decls) {
        if (IsPublic(member_decl)) {
            VisitNode(member_decl, depth + 1);
        }
    }

    if (IsAbstractOrOpen(decl)) {
        abstract_class.insert(decl->identifier.Val());
    }

    if (!IsPublic(decl)) {
        function_map.insert(decl->identifier.Val());
    }
}

void HeadFileVisitor::VisitEnumConstructor(EnumDecl *enum_decl)
{
    const std::vector<OwnedPtr<Decl>> &constructors = enum_decl->constructors;
    std::function<void(const OwnedPtr<Decl> &, size_t)> visit_constructor =
        [&constructors, &enum_decl, this, &visit_constructor](const OwnedPtr<Decl> &cons, size_t index) {
            auto macroCall = GetCurMacro(*cons.get().get());
            auto comments = FindCommentsInMacroCall(cons, macroCall);
            for (auto &comment : comments) {
                cons->comments.leadingComments.push_back(comment);
            }
        };

    for (size_t i = 0; i < constructors.size(); ++i) {
        auto &cons = constructors[i];
        visit_constructor(cons, i);
    }
}

void HeadFileVisitor::VisitExtendDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto extend_decl = dynamic_cast<ExtendDecl *>(decl.get().get());
    auto &member_decls = extend_decl->GetMemberDecls();

    auto receiver_name = extend_decl->extendedType->ToString();
    if (function_map.count(receiver_name) != 0) {
        removed_extend.insert(receiver_name);
    } else {
        member_decls.erase(std::remove_if(member_decls.begin(),
            member_decls.end(),
            [this, &receiver_name](auto &x) {
                return !(IsPublic(x) || abstract_class.count(receiver_name) != 0 && IsProtected(x));
            }),
            member_decls.end());

        for (const auto &member_decl : member_decls) {
            if (IsPublic(member_decl)) {
                VisitNode(member_decl, depth + 1);
            }
        }
    }
}

void HeadFileVisitor::VisitFuncDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto func_decl = dynamic_cast<FuncDecl *>(decl.get().get());
    if (depth == 0 && !IsPublic(decl)) {
        return;
    }

    if (from_interface) {
        return;
    }

    OwnedPtr<FuncBody> &func_body = func_decl->funcBody;
    OwnedPtr<Block> &func_block = func_body->body;
    if (func_block)
        func_block->body = std::vector<OwnedPtr<Node>>();
}

void HeadFileVisitor::VisitVarDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    if (decl->TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }
}

void HeadFileVisitor::VisitPropDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto prop_decl = dynamic_cast<PropDecl *>(decl.get().get());
    prop_decl->getters = std::vector<OwnedPtr<FuncDecl>>();
    prop_decl->setters = std::vector<OwnedPtr<FuncDecl>>();
}

void HeadFileVisitor::VisitPrimaryCtorDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto primary_ctor_decl = dynamic_cast<PrimaryCtorDecl *>(decl.get().get());
}

void HeadFileVisitor::VisitMacroDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto macro_decl = dynamic_cast<MacroDecl *>(decl.get().get());
    OwnedPtr<FuncBody> &func_body = macro_decl->funcBody;
    if (func_body) {
        OwnedPtr<Block> &func_block = func_body->body;
        if (func_block)
            func_block->body = std::vector<OwnedPtr<Node>>();
    }
}

void HeadFileVisitor::VisitMacroExpandDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{
    auto macro_expand_decl = dynamic_cast<MacroExpandDecl *>(decl.get().get());
    auto &macro_invocation = macro_expand_decl->invocation;
    OwnedPtr<Decl> &owned_decl = macro_invocation.decl;

    if (owned_decl->astKind == ASTKind::TYPE_ALIAS_DECL) {
        VisitNode(owned_decl, depth, from_interface);
    }
    VisitMacroExpandString(decl);
    VisitNode(owned_decl, depth, from_interface);
}

void HeadFileVisitor::VisitMacroExpandString(const OwnedPtr<Decl> &decl)
{
    auto macro_expand_decl = dynamic_cast<MacroExpandDecl *>(decl.get().get());
    auto &macro_invocation = macro_expand_decl->invocation;
    OwnedPtr<Decl> &owned_decl = macro_invocation.decl;
}

void HeadFileVisitor::VisitTypealiasDecl(const OwnedPtr<Decl> &decl, const int &depth, const bool &from_interface)
{}
/* -- end of visit */

void HeadFileVisitor::VisitTypealias()
{}

auto HeadFileVisitor::IsPublic(const OwnedPtr<Decl> &decl) const -> bool
{
    if (decl->astKind == ASTKind::MACRO_EXPAND_DECL) {
        auto macro_expand_decl = dynamic_cast<MacroExpandDecl *>(decl.get().get());
        auto &macro_invocation = macro_expand_decl->invocation;
        OwnedPtr<Decl> &owned_decl = macro_invocation.decl;
        return IsPublic(owned_decl);
    }
    if (decl->astKind == ASTKind::EXTEND_DECL) {
        return true;
    }
    auto &modifiers = decl->modifiers;

    return std::find_if(modifiers.begin(), modifiers.end(), [](const struct Modifier &modifier) {
        return modifier.ToString() == "public" or modifier.ToString() == "sealed";
    }) != modifiers.end();
}

auto HeadFileVisitor::IsProtected(const OwnedPtr<Decl> &decl) const -> bool
{
    if (decl->astKind == ASTKind::MACRO_EXPAND_DECL) {
        auto macro_expand_decl = dynamic_cast<MacroExpandDecl *>(decl.get().get());
        auto &macro_invocation = macro_expand_decl->invocation;
        OwnedPtr<Decl> &owned_decl = macro_invocation.decl;
        return IsProtected(owned_decl);
    }
    auto &modifiers = decl->modifiers;
    return std::find_if(modifiers.begin(), modifiers.end(), [](const struct Modifier &modifier) {
        return modifier.ToString() == "protected";
    }) != modifiers.end();
}

auto HeadFileVisitor::IsPrivate(const OwnedPtr<Decl> &decl) const -> bool
{
    if (decl->astKind == ASTKind::MACRO_EXPAND_DECL) {
        auto macro_expand_decl = dynamic_cast<MacroExpandDecl *>(decl.get().get());
        auto &macro_invocation = macro_expand_decl->invocation;
        OwnedPtr<Decl> &owned_decl = macro_invocation.decl;
        return IsProtected(owned_decl);
    }
    auto &modifiers = decl->modifiers;
    return std::find_if(modifiers.begin(), modifiers.end(), [](const struct Modifier &modifier) {
        return modifier.ToString() == "private";
    }) != modifiers.end();
}

auto HeadFileVisitor::GetModifierString(const OwnedPtr<Decl> &decl) const -> std::string
{
    auto &modifiers = decl->modifiers;
    if (modifiers.empty()) {
        return "";
    }
    std::string modifier_string{""};
    for (auto &modifier : modifiers) {
        modifier_string += modifier.ToString() + " ";
    }
    return modifier_string;
}

auto HeadFileVisitor::GetSrcIndentifierString(const OwnedPtr<Decl> &decl) const -> std::string
{
    return (decl->identifier).GetRawText();
}

auto HeadFileVisitor::GetCommentString(const OwnedPtr<Node> &node) const -> std::vector<std::string>
{
    std::string comment_string;
    auto lead_comments_group = node->comments.leadingComments;
    for (auto &lead_comment : lead_comments_group) {
        for (auto &comment : lead_comment.cms) {
            comment_string += comment.info.Value() + "\n";
        }
    }
    return CJHeadUtil::SplitStringTolines(comment_string);
}

auto HeadFileVisitor::GetCommentString(const Ptr<Node> &node) const -> std::vector<std::string>
{
    std::string comment_string;
    auto lead_comments_group = node->comments.leadingComments;
    for (auto &lead_comment : lead_comments_group) {
        for (auto &comment : lead_comment.cms) {
            comment_string += comment.info.Value();
        }
    }
    return CJHeadUtil::SplitStringTolines(comment_string);
}

auto HeadFileVisitor::GetGeneriParam(const OwnedPtr<Decl> &decl) const -> std::string
{
    return "";
}

auto HeadFileVisitor::GetGenericCst(const OwnedPtr<Decl> &decl) const -> std::string
{
    return "";
}

auto HeadFileVisitor::GetInheritedTypes(const OwnedPtr<Decl> &decl) const -> std::string
{
    auto inherited_decl = dynamic_cast<InheritableDecl *>(decl.get().get());
    std::vector<OwnedPtr<Type>> &inherited_types = inherited_decl->inheritedTypes;
    std::string fathers{"<:"};
    if (inherited_types.empty()) {
        return "";
    }
    if (!inherited_types.empty()) {
        std::string temp{""};
        for (auto &inherited_type : inherited_types) {
            if (temp != "") {
                temp += "&" + inherited_type->ToString();
            } else {
                temp += inherited_type->ToString();
            }
        }
        fathers += temp;
    }
    return fathers;
}

auto HeadFileVisitor::GetAnnotationString(const OwnedPtr<Decl> &decl) const -> std::string
{
    std::vector<OwnedPtr<Annotation>> &annotations = decl->annotations;
    if (annotations.empty()) {
        return "";
    }
    std::string annotation_string{""};

    for (const auto &anno : annotations) {
        annotation_string += source_manager_.GetContentBetween(anno->begin, anno->end);
        OwnedPtr<Expr> &expr = anno->condExpr;
        if (expr) {
            annotation_string += "[" + source_manager_.GetContentBetween(expr->begin, expr->end) + "]";
        }
        annotation_string += "\n";
    }
    if (annotation_string.back() == '\n') {
        annotation_string.pop_back();
    }
    return annotation_string;
}

auto HeadFileVisitor::GetSignature(const OwnedPtr<Decl> &decl) const -> std::string
{
    if (decl->astKind == ASTKind::FUNC_DECL) {
        auto func_decl = dynamic_cast<FuncDecl *>(decl.get().get());
        OwnedPtr<FuncBody> &func_body = func_decl->funcBody;
        OwnedPtr<Block> &func_block = func_body->body;
        if (func_block) {
            std::string func_sig = source_manager_.GetContentBetween(decl->begin, func_block->leftCurlPos);
            return func_sig.back() == ' ' ? std::string(func_sig.begin(), func_sig.end() - 1) : func_sig;
        }
        return source_manager_.GetContentBetween(decl->begin, decl->end);
    }
    if (decl->astKind == ASTKind::PRIMARY_CTOR_DECL) {
        auto func_decl = dynamic_cast<PrimaryCtorDecl *>(decl.get().get());
        OwnedPtr<FuncBody> &func_body = func_decl->funcBody;
        OwnedPtr<Block> &func_block = func_body->body;
        if (func_block) {
            std::string func_sig = source_manager_.GetContentBetween(decl->begin, func_block->leftCurlPos);
            return func_sig.back() == ' ' ? std::string(func_sig.begin(), func_sig.end() - 1) : func_sig;
        }
        return source_manager_.GetContentBetween(decl->begin, decl->end);
    }
    if (decl->astKind == ASTKind::VAR_DECL) {
        return source_manager_.GetContentBetween(decl->begin, decl->end);
    }
    std::string content = source_manager_.GetContentBetween(decl->begin, decl->end);
    size_t pos = content.find("{");
    std::string signature{content};
    if (pos != std::string::npos) {
        signature = content.substr(0, pos);
    }
    if (signature.back() == ' ') {
        signature.pop_back();
    }
    return signature;
}

auto HeadFileVisitor::getIndent(size_t indet) const -> std::string
{
    return std::string(indet * NUM_SPACES, ' ');
}

void HeadFileVisitor::addLineToHeader(const std::string &str, const bool &is_comment)
{
    auto new_line = getIndent(indent_) + str + '\n';
    header_file_.emplace_back(new_line, is_comment);
}

auto HeadFileVisitor::GetHeaderFile() -> HeadFileFormat
{
    return header_file_;
}

auto HeadFileVisitor::GetFirstComment() -> std::string
{
    return first_comment_;
}

auto HeadFileVisitor::GetImportInfo() -> std::vector<std::string>
{
    return import_info_;
}

void HeadFileVisitor::incIndent()
{
    ++indent_;
}

void HeadFileVisitor::decIndent()
{
    --indent_;
}

}  // namespace CJHead
