// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <vector>
#include <map>
#include <algorithm>
#include <unordered_map>
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"
#include "Format/NodeFomatter/Node/FileFormatter.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

namespace {
std::vector<Cangjie::Position> GetWhenOrForeignPositions(const AST::Decl& decl)
{
    std::vector<Cangjie::Position> result;
    for (auto& ann : decl.annotations) {
        result.push_back(ann->begin);
    }
    for (auto& mod : decl.modifiers) {
        if (mod.modifier == TokenKind::FOREIGN) {
            result.push_back(mod.begin);
        }
    }
    return result;
}

void AddPackageSpec(Doc& doc, const Cangjie::AST::File& file, ASTToFormatSource& astToFormatSource, int level)
{
    if (file.package) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(file.package.get(), level));
    }
}

void AddImportSpecs(Doc& doc, const Cangjie::AST::File& file, ASTToFormatSource& astToFormatSource, int level)
{
    if (file.imports.empty()) {
        return;
    }

    if (file.package) {
        doc.members.emplace_back(DocType::SEPARATE, level, "");
        doc.members.emplace_back(DocType::SEPARATE, level, "");
    }

    for (unsigned int i = 0; i < file.imports.size(); i++) {
        auto importSpec = file.imports[i].get();
        if (i != 0 && !importSpec->TestAttr(Attribute::COMPILER_ADD)) {
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(importSpec, level));
    }
}

void AddBlockPrefix(const OwnedPtr<Decl>& decl, std::vector<Position>& beforeBlockAnnotationsPositions, Doc& doc,
    ASTToFormatSource& astToFormatSource, int level)
{
    std::vector<Ptr<Annotation>> annotationsToRender;
    for (auto& ann : decl->annotations) {
        for (auto beginPos : beforeBlockAnnotationsPositions) {
            if (ann->begin == beginPos) {
                annotationsToRender.push_back(ann.get());
            }
        }
    }

    std::set<Modifier> modifiersToRender;
    for (auto& mod : decl->modifiers) {
        for (auto beginPos : beforeBlockAnnotationsPositions) {
            if (mod.begin == beginPos) {
                (void)modifiersToRender.insert(mod);
            }
        }
    }

    for (auto it = annotationsToRender.begin(); it != annotationsToRender.end(); ++it) {
        if (it != annotationsToRender.begin()) {
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(*it));
    }
    if (!annotationsToRender.empty() && !modifiersToRender.empty()) {
        doc.members.emplace_back(DocType::LINE, level, "");
    }

    astToFormatSource.AddModifier(doc, modifiersToRender, level);

    if (modifiersToRender.empty()) {
        doc.members.emplace_back(DocType::STRING, level, " ");
    }

    doc.members.emplace_back(DocType::STRING, level, "{");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
}

void AddBlockSuffix(Doc& doc, int level)
{
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}

void ShouldLastBlockADDSpearate(
    Doc& doc, int level, const File& file, std::vector<OwnedPtr<Cangjie::AST::Decl>>::const_iterator it)
{
    if (it->get()->astKind == AST::ASTKind::VAR_DECL && it != file.decls.end() - 1 &&
        std::next(it)->get()->astKind == AST::ASTKind::VAR_DECL) {
        doc.members.emplace_back(DocType::SEPARATE, level, "");
    }
}

void EraseModifiersAndAnnotationsForDecl(
    const OwnedPtr<Decl>& decl, std::vector<Position>& beforeBlockAnnotationsPositions)
{
    for (auto& beginPos : beforeBlockAnnotationsPositions) {
        auto& annotations = decl->annotations;
        auto annotationToErase = std::find_if(
            annotations.begin(), annotations.end(), [beginPos](auto& ann) { return ann->begin == beginPos; });
        if (annotationToErase != annotations.end()) {
            (void)annotations.erase(annotationToErase);
        }

        auto& modifiers = decl->modifiers;
        auto modifierToErase =
            std::find_if(modifiers.begin(), modifiers.end(), [beginPos](auto& m) { return m.begin == beginPos; });
        if (modifierToErase != modifiers.end()) {
            (void)modifiers.erase(modifierToErase);
        }
    }
}
} // namespace

void FileFormatter::AddFile(Doc& doc, const Cangjie::AST::File& file, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    AddPackageSpec(doc, file, astToFormatSource, level);
    AddImportSpecs(doc, file, astToFormatSource, level);
    std::unordered_map<Cangjie::Position, int, PositionHasher> modifierOrAnnoToPosMap;
    SetModifierOrAnnoToPosMap(file, modifierOrAnnoToPosMap);
    if (file.package || !file.imports.empty()) {
        doc.members.emplace_back(DocType::SEPARATE, level, "");
        doc.members.emplace_back(DocType::SEPARATE, level, "");
    }
    auto preDeclIsVar = false;
    auto mapCopy(modifierOrAnnoToPosMap);
    for (auto it = file.decls.begin(); it != file.decls.end(); ++it) {
        bool isFirstInBlock = false;
        bool isLastInBlock = false;
        std::vector<Cangjie::Position> beforeBlockAnnotationsPositions;
        for (auto pos : GetWhenOrForeignPositions(*it->get())) {
            if (modifierOrAnnoToPosMap.find(pos) != modifierOrAnnoToPosMap.end() && modifierOrAnnoToPosMap[pos] > 1) {
                beforeBlockAnnotationsPositions.emplace_back(pos);
                isFirstInBlock = (mapCopy[pos] == modifierOrAnnoToPosMap[pos]);
                mapCopy[pos]--;
                isLastInBlock = (mapCopy[pos] == 0);
            }
        }
        auto declarationLevel = beforeBlockAnnotationsPositions.empty() ? level : level + 1;
        auto addSeparate = [&it, &doc, &level]() {
            if (it->get()->astKind != AST::ASTKind::VAR_DECL)
                doc.members.emplace_back(DocType::SEPARATE, level, "");
        };
        if (*it != file.decls.front()) {
            if (preDeclIsVar) {
                addSeparate();
                doc.members.emplace_back(DocType::LINE, isFirstInBlock ? level : declarationLevel, "");
            } else {
                doc.members.emplace_back(DocType::SEPARATE, level, "");
                doc.members.emplace_back(DocType::LINE, isFirstInBlock ? level : declarationLevel, "");
            }
        }
        if (isFirstInBlock)
            AddBlockPrefix(*it, beforeBlockAnnotationsPositions, doc, astToFormatSource, level);
        EraseModifiersAndAnnotationsForDecl(*it, beforeBlockAnnotationsPositions);
        preDeclIsVar = it->get()->astKind == ASTKind::VAR_DECL;
        doc.members.emplace_back(astToFormatSource.ASTToDoc(*it, declarationLevel));
        if (isLastInBlock) {
            AddBlockSuffix(doc, level);
            ShouldLastBlockADDSpearate(doc, level, file, it);
        }
    }
}

void FileFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto file = As<ASTKind::FILE>(node);
    AddFile(doc, *file, level);
}

void FileFormatter::SetModifierOrAnnoToPosMap(
    const Cangjie::AST::File& file, std::unordered_map<Cangjie::Position, int, PositionHasher>& modifierOrAnnoToPosMap)
{
    for (auto& decl : file.decls) {
        for (auto pos : GetWhenOrForeignPositions(*decl)) {
            ++modifierOrAnnoToPosMap[pos];
        }
    }
}
} // namespace Cangjie::Format
