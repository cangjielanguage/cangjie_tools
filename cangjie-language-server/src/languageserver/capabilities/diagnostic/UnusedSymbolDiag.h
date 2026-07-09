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
#include "../../index/MemIndex.h"
#include "../../index/Ref.h"
#include "../../index/Relation.h"
#include "../../index/Symbol.h"
#include "LSPDiagObserver.h"
#include "cangjie/AST/Node.h"
#include "cangjie/AST/Walker.h"

namespace ark {

/// Macro names whose decorated class/struct (and all their members)
/// should be excluded from unused symbol reporting.
inline const std::unordered_set<std::string> EXCLUDED_MACRO_NAMES = {
    "Entry", "Component", "Test"
};

/// Target ASTKinds for unused symbol detection at the index level.
inline const std::unordered_set<Cangjie::AST::ASTKind> UNUSED_CHECK_KINDS = {
    Cangjie::AST::ASTKind::FUNC_DECL,
    Cangjie::AST::ASTKind::VAR_DECL,
    Cangjie::AST::ASTKind::LAMBDA_EXPR,
    Cangjie::AST::ASTKind::CLASS_DECL,
    Cangjie::AST::ASTKind::STRUCT_DECL,
    Cangjie::AST::ASTKind::ENUM_DECL,
    Cangjie::AST::ASTKind::INTERFACE_DECL,
    Cangjie::AST::ASTKind::TYPE_ALIAS_DECL,
};

class UnusedSymbolDiag {
public:
    /**
     * Analyze global/member symbols from the index to detect unused symbols.
     * Queries the SymbolIndex per-file for symbols, references, and relations.
     *
      * @param index         The symbol index (MemIndex or BackgroundIndexDB)
      * @param pkgName       The full package name for symbol scoping
      * @param filePath      The file to produce diagnostics for
      * @param package       The AST package (for macro detection)
      * @param timeoutMs     Max analysis time in ms, -1 for unlimited
      * @return vector of DiagnosticToken for unused symbols
      */
    static std::vector<DiagnosticToken> Analyze(
        const lsp::SymbolIndex& index,
        const std::string& pkgName,
        const std::string& filePath,
        const Cangjie::AST::Package* package
    );

    /**
     * Analyze local variables and non-named function parameters
     * that are not covered by the index system.
     *
     * @param file      The AST File node
     * @param package   The AST Package node (used as root for FindDeclUsage)
     * @param index     Optional symbol index for cross-package reference check.
     *                  When non-null, enum variants are also checked against the
     *                  index to avoid false positives from cross-package usage.
     * @return vector of DiagnosticToken for unused local symbols
     */
    static std::vector<DiagnosticToken> AnalyzeLocalSymbols(
        const Cangjie::AST::File& file,
        Cangjie::AST::Package& package,
        const lsp::SymbolIndex* index = nullptr
    );

    /// Create an "unused" DiagnosticToken with HINT severity and Unnecessary tag.
    static DiagnosticToken CreateUnusedDiag(
        const std::string& name,
        const lsp::SymbolLocation& location,
        const std::string& symbolKindDesc
    );

private:
    /// Collect SymbolIDs of classes/structs that are decorated with @Entry/@Component.
    static std::unordered_set<lsp::SymbolID> CollectExcludedMacroDecls(
        const Cangjie::AST::Package* package
    );

    /// Check if a symbol should be excluded from unused detection.
    static bool ShouldExclude(
        const lsp::Symbol& symbol,
        const lsp::SymbolIndex& index,
        const std::unordered_set<lsp::SymbolID>& excludedMacroDecls
    );

    /// Get a human-readable description of the symbol kind.
    static std::string GetKindDescription(Cangjie::AST::ASTKind kind);
};

} // namespace ark

#endif // LSPSERVER_UNUSED_SYMBOL_DIAG_H
