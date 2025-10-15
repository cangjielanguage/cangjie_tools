// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGENU01.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

/*
 * The function which help to find the constructors of enum and add constructs into enum set.
 */
void StructuralRuleGENU01::DuplicateNameFinderHelper(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            // 识别程序中枚举声明语句
            [this](const EnumDecl &enumDecl) {
                for (auto &i : enumDecl.constructors) {
                    (void)enumSet.emplace(
                        i->identifier.Val(), std::make_pair(i->identifier.Begin(), i->identifier.End()));
                }
                return VisitAction::SKIP_CHILDREN;
            },
            // 其他语句放在这里
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

/*
 * The function that check the conflict of top-level object declaration with enum constructors.
 */
void StructuralRuleGENU01::DiagnosticsFunc(const Decl &decl)
{
    if (!enumSet.empty()) {
        for (auto &item : enumSet) {
            if (item.first == decl.identifier) {
                Diagnose(decl.identifier.Begin(), decl.identifier.End(),
                    CodeCheckDiagKind::G_ENU_01_0_duplicate_name_information, decl.identifier.Val().c_str());
                Diagnose(item.second.first, item.second.second,
                    CodeCheckDiagKind::G_ENU_01_1_duplicate_name_information, item.first.c_str());
            }
        }
    }
}

void StructuralRuleGENU01::DuplicateNameFinder(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    match (*node)(
        [this](const Package &package) {
            for (auto &it : package.files) {
                DuplicateNameFinder(it.get());
            }
        },
        [this](const File &file) {
            for (auto &it : file.decls) {
                DuplicateNameFinder(it.get());
            }
        },
        // 识别程序中类声明语句
        [this](const ClassDecl &classDecl) {
            DiagnosticsFunc((Decl &)classDecl);
        },
        // 识别程序中接口声明语句
        [this](const InterfaceDecl &interfaceDecl) {
            DiagnosticsFunc((Decl &)interfaceDecl);
        },
        // 识别程序中函数声明语句
        [this](const FuncDecl &funcDecl) { DiagnosticsFunc((Decl &)funcDecl); },
        // 识别程序中变量声明语句
        [this](const VarDecl &varDecl) { DiagnosticsFunc((Decl &)varDecl); },
        // 识别程序中枚举声明语句
        [this](const EnumDecl &enumDecl) { DiagnosticsFunc((Decl &)enumDecl); },
        // 识别程序中Struct声明语句
        [this](const StructDecl &structDecl) { DiagnosticsFunc((Decl &)structDecl); },
        [this](const MacroDecl &macroDecl) { DiagnosticsFunc((Decl &)macroDecl); },
        // 识别程序中TypeAlias声明语句
        [this](const TypeAliasDecl &typeAliasDecl) { DiagnosticsFunc((Decl &)typeAliasDecl); });
}

void StructuralRuleGENU01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    DuplicateNameFinderHelper(node);
    DuplicateNameFinder(node);
}
} // namespace Cangjie::CodeCheck
