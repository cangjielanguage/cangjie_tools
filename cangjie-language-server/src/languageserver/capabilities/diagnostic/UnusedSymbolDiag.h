// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_UNUSED_SYMBOL_DIAG_H
#define LSPSERVER_UNUSED_SYMBOL_DIAG_H

#include <string>
#include <unordered_set>
#include <vector>
#include "../../common/Callbacks.h"
#include "../../common/PositionResolver.h"
#include "../../index/Ref.h"
#include "../../index/Relation.h"
#include "../../index/Symbol.h"
#include "LSPDiagObserver.h"
#include "cangjie/AST/Node.h"
#include "cangjie/AST/Walker.h"

namespace ark {

/// ArkUI component lifecycle method names that should not be reported as unused
/// when they belong to a class/struct decorated with @Entry or @Component.
inline const std::unordered_set<std::string> ARKUI_LIFECYCLE_METHODS = {
    "aboutToAppear", "aboutToDisappear",
    "onPageShow", "onPageHide", "onBackPress",
    "onMeasureSize", "onPlaceChildren",
    "aboutToReuse", "aboutToRecycle",
    "onWillApplyTheme", "build"
};

/// Macro names whose decorated class/struct (and their lifecycle methods)
/// should be excluded from unused symbol reporting.
inline const std::unordered_set<std::string> EXCLUDED_MACRO_NAMES = {
    "Entry", "Component"
};

/// Target ASTKinds for unused symbol detection at the index level.
inline const std::unordered_set<Cangjie::AST::ASTKind> UNUSED_CHECK_KINDS = {
    Cangjie::AST::ASTKind::FUNC_DECL,
    Cangjie::AST::ASTKind::VAR_DECL,
    Cangjie::AST::ASTKind::CLASS_DECL,
    Cangjie::AST::ASTKind::STRUCT_DECL,
    Cangjie::AST::ASTKind::ENUM_DECL,
    Cangjie::AST::ASTKind::INTERFACE_DECL,
    Cangjie::AST::ASTKind::TYPE_ALIAS_DECL,
};

class UnusedSymbolDiag {
public:
    /**
     * Analyze collected symbols/refs from SymbolCollector to detect unused
     * global/member symbols. Returns diagnostic tokens for a specific file.
     *
     * @param symbols       All symbols in the package
     * @param refs          All references in the package
     * @param relations     All relations in the package (for override detection)
     * @param filePath      The file to produce diagnostics for
     * @param package       The AST package (for macro detection)
     * @return vector of DiagnosticToken for unused symbols
     */
    static std::vector<DiagnosticToken> Analyze(
        const std::vector<lsp::Symbol>& symbols,
        const std::map<lsp::SymbolID, std::vector<lsp::Ref>>& refs,
        const std::vector<lsp::Relation>& relations,
        const std::string& filePath,
        const Cangjie::AST::Package* package
    );

    /**
     * Analyze local variables and non-named function parameters
     * that are not covered by the index system.
     *
     * @param file      The AST File node
     * @param package   The AST Package node (used as root for FindDeclUsage)
     * @return vector of DiagnosticToken for unused local symbols
     */
    static std::vector<DiagnosticToken> AnalyzeLocalSymbols(
        const Cangjie::AST::File& file,
        Cangjie::AST::Package& package
    );

    /// Check if a symbol has any REFERENCE-kind ref (not just DEFINITION).
    static bool HasReference(
        lsp::SymbolID symId,
        const std::map<lsp::SymbolID, std::vector<lsp::Ref>>& refs
    );

    /// Create an "unused" DiagnosticToken with HINT severity and Unnecessary tag.
    static DiagnosticToken CreateUnusedDiag(
        const std::string& name,
        const lsp::SymbolLocation& location,
        const std::string& symbolKindDesc
    );

private:
    /// Check if a symbol is an override (has a RIDDEND_BY relation as object).
    static bool IsOverride(
        lsp::SymbolID symId,
        const std::vector<lsp::Relation>& relations
    );

    /// Collect SymbolIDs of classes/structs that are decorated with @Entry/@Component.
    static std::unordered_set<lsp::SymbolID> CollectExcludedMacroDecls(
        const Cangjie::AST::Package* package
    );

    /// Check if a symbol should be excluded from unused detection.
    static bool ShouldExclude(
        const lsp::Symbol& symbol,
        const std::vector<lsp::Relation>& relations,
        const std::unordered_set<lsp::SymbolID>& excludedMacroDecls
    );

    /// Get a human-readable description of the symbol kind.
    static std::string GetKindDescription(Cangjie::AST::ASTKind kind);
};

} // namespace ark

#endif // LSPSERVER_UNUSED_SYMBOL_DIAG_H
