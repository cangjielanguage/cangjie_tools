// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "UnusedSymbolDiag.h"
#include "../../common/FindDeclUsage.h"
#include "../../common/Utils.h"

namespace ark {
using namespace Cangjie;
using namespace AST;
using namespace lsp;

// ---------------------------------------------------------------------------
// Helper: Check if a symbol has any REFERENCE-kind ref
// ---------------------------------------------------------------------------
bool UnusedSymbolDiag::HasReference(
    SymbolID symId,
    const std::map<SymbolID, std::vector<Ref>>& refs)
{
    auto it = refs.find(symId);
    if (it == refs.end()) {
        return false;
    }
    for (const auto& ref : it->second) {
        if (static_cast<uint8_t>(ref.kind & RefKind::REFERENCE) != 0) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Helper: Check if a symbol is an override method (RIDDEND_BY as object)
// ---------------------------------------------------------------------------
bool UnusedSymbolDiag::IsOverride(
    SymbolID symId,
    const std::vector<Relation>& relations)
{
    for (const auto& rel : relations) {
        if (rel.predicate == RelationKind::RIDDEND_BY && rel.object == symId) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Helper: Get the SymbolID of the containing (outer) decl for a member symbol
// from CONTAINED_BY relations.
// ---------------------------------------------------------------------------
static SymbolID GetContainerID(
    SymbolID symId,
    const std::vector<Relation>& relations)
{
    for (const auto& rel : relations) {
        if (rel.predicate == RelationKind::CONTAINED_BY && rel.subject == symId) {
            return rel.object;
        }
    }
    return INVALID_SYMBOL_ID;
}

// ---------------------------------------------------------------------------
// Helper: Check if any child constructor of a class/struct has references.
// Uses CONTAINED_BY relations to find constructor children, then checks refs.
// ---------------------------------------------------------------------------
static bool HasConstructorReference(
    SymbolID symId,
    const std::vector<Relation>& relations,
    const std::map<SymbolID, std::vector<Ref>>& refs)
{
    for (const auto& rel : relations) {
        if (rel.predicate == RelationKind::CONTAINED_BY && rel.object == symId) {
            if (UnusedSymbolDiag::HasReference(rel.subject, refs)) {
                return true;
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Helper: Try to add a macro-decorated decl symbol to the result set.
// Checks if the macro is in EXCLUDED_MACRO_NAMES (@Entry/@Component) and
// the declaration is a class/struct, then adds its SymbolID to result.
// ---------------------------------------------------------------------------
static void TryAddMacroDeclSymbol(
    MacroExpandDecl* macroExpandDecl,
    ASTKind astKind,
    Decl* decl,
    std::unordered_set<SymbolID>& result)
{
    if (!macroExpandDecl || !macroExpandDecl->invocation.decl) {
        return;
    }
    const auto& macroName = macroExpandDecl->invocation.macroCallDiagInfo.identifier;
    if (EXCLUDED_MACRO_NAMES.count(macroName) == 0) {
        return;
    }
    if (astKind != ASTKind::CLASS_DECL && astKind != ASTKind::STRUCT_DECL) {
        return;
    }
    if (!decl) {
        return;
    }
    auto symId = GetSymbolId(*decl);
    if (symId != INVALID_SYMBOL_ID) {
        (void)result.emplace(symId);
    }
}

// ---------------------------------------------------------------------------
// Collect SymbolIDs of class/struct declarations decorated with @Entry/@Component.
// Walks the file-level declarations of each file in the package.
// ---------------------------------------------------------------------------
std::unordered_set<SymbolID> UnusedSymbolDiag::CollectExcludedMacroDecls(
    const Package* package)
{
    std::unordered_set<SymbolID> result;
    if (!package) {
        return result;
    }

    for (auto& file : package->files) {
        if (!file) {
            continue;
        }
        for (auto& toplevelDecl : file->decls) {
            if (!toplevelDecl) {
                continue;
            }
            if (toplevelDecl->astKind == ASTKind::MACRO_EXPAND_DECL) {
                auto macroExpandDecl = dynamic_cast<MacroExpandDecl*>(toplevelDecl.get().get());
                auto innerDecl = macroExpandDecl ? macroExpandDecl->invocation.decl.get() : nullptr;
                TryAddMacroDeclSymbol(macroExpandDecl,
                    innerDecl ? innerDecl->astKind : static_cast<ASTKind>(0),
                    innerDecl, result);
            }
            if (toplevelDecl->curMacroCall && !toplevelDecl->isInMacroCall) {
                auto* macroExpandDecl = dynamic_cast<MacroExpandDecl*>(toplevelDecl->curMacroCall.get());
                TryAddMacroDeclSymbol(macroExpandDecl, toplevelDecl->astKind,
                    DynamicCast<Decl*>(toplevelDecl.get()), result);
            }
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Helper: Check if symbol should be excluded based on type or modifier.
// Handles simple exclusion rules that don't require relation/macro analysis.
// ---------------------------------------------------------------------------
static bool ShouldExcludeByTypeOrModifier(const Symbol& symbol)
{
    if (symbol.modifier == Modifier::PUBLIC || symbol.modifier == Modifier::PROTECTED) {
        return true;
    }

    if (symbol.symInfo.kind == lsp::SymbolKind::CONSTRUCTOR ||
        symbol.symInfo.kind == lsp::SymbolKind::DESTRUCTOR) {
        return true;
    }

    if (symbol.kind == ASTKind::MAIN_DECL ||
        (symbol.kind == ASTKind::FUNC_DECL && symbol.name == "main")) {
        return true;
    }

    if (!symbol.curMacroCall.IsZeroLoc()) {
        return true;
    }

    if (symbol.isMemberParam) {
        return true;
    }

    if (symbol.isCjoSym) {
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Determine whether a symbol should be excluded from unused detection.
// ---------------------------------------------------------------------------
bool UnusedSymbolDiag::ShouldExclude(
    const Symbol& symbol,
    const std::vector<Relation>& relations,
    const std::unordered_set<SymbolID>& excludedMacroDecls)
{
    if (ShouldExcludeByTypeOrModifier(symbol)) {
        return true;
    }

    if (symbol.kind == ASTKind::FUNC_DECL || symbol.kind == ASTKind::PROP_DECL) {
        if (IsOverride(symbol.id, relations)) {
            return true;
        }
    }

    if ((symbol.kind == ASTKind::CLASS_DECL || symbol.kind == ASTKind::STRUCT_DECL) &&
        excludedMacroDecls.count(symbol.id) > 0) {
        return true;
    }

    if (symbol.kind == ASTKind::FUNC_DECL) {
        if (ARKUI_LIFECYCLE_METHODS.count(symbol.name) > 0) {
            SymbolID containerId = GetContainerID(symbol.id, relations);
            if (containerId != INVALID_SYMBOL_ID && excludedMacroDecls.count(containerId) > 0) {
                return true;
            }
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// Get a human-readable description for an ASTKind
// ---------------------------------------------------------------------------
std::string UnusedSymbolDiag::GetKindDescription(ASTKind kind)
{
    switch (kind) {
        case ASTKind::FUNC_DECL:
            return "Function";
        case ASTKind::VAR_DECL:
            return "Variable";
        case ASTKind::CLASS_DECL:
            return "Class";
        case ASTKind::STRUCT_DECL:
            return "Struct";
        case ASTKind::ENUM_DECL:
            return "Enum";
        case ASTKind::INTERFACE_DECL:
            return "Interface";
        default:
            return "Symbol";
    }
}

// ---------------------------------------------------------------------------
// Create a DiagnosticToken for an unused symbol
// ---------------------------------------------------------------------------
DiagnosticToken UnusedSymbolDiag::CreateUnusedDiag(
    const std::string& name,
    const SymbolLocation& location,
    const std::string& symbolKindDesc)
{
    DiagnosticToken diagToken;
    ark::Range lspRange = TransformFromChar2IDE({
        {location.begin.fileID, location.begin.line, location.begin.column},
        {location.end.fileID, location.end.line, location.end.column}
    });
    diagToken.range = {
        {lspRange.start.fileID, lspRange.start.line, lspRange.start.column},
        {lspRange.end.fileID, lspRange.end.line, lspRange.end.column}
    };
    diagToken.severity = static_cast<int>(DiagLSPSeverity::HINT);  // 4 = Hint
    diagToken.tags = {1};  // DiagnosticTag.Unnecessary = 1 (editor renders as faded/grey)
    diagToken.source = "Cangjie";
    diagToken.message = symbolKindDesc + " '" + name + "' is declared but never used";
    diagToken.code = 0;
    diagToken.diaFix = DiagFix{false, false, true};  // Enable removeUnusedSymbol QuickFix
    return diagToken;
}

// ---------------------------------------------------------------------------
// Main analysis: detect unused global/member symbols from the index
// ---------------------------------------------------------------------------
std::vector<DiagnosticToken> UnusedSymbolDiag::Analyze(
    const std::vector<Symbol>& symbols,
    const std::map<SymbolID, std::vector<Ref>>& refs,
    const std::vector<Relation>& relations,
    const std::string& filePath,
    const Package* package)
{
    std::vector<DiagnosticToken> diagnostics;

    // Collect class/struct SymbolIDs decorated with @Entry/@Component
    auto excludedMacroDecls = CollectExcludedMacroDecls(package);

    for (const auto& sym : symbols) {
        // 1. Only process symbols defined in the target file
        if (sym.location.fileUri != filePath) {
            continue;
        }

        // 2. Only process target ASTKinds (func, var, class, struct, enum, interface)
        if (UNUSED_CHECK_KINDS.count(sym.kind) == 0) {
            continue;
        }

        // 3. Skip symbols at zero locations (implicit/generated)
        if (sym.location.IsZeroLoc()) {
            continue;
        }

        // 4. Skip invalid symbol IDs
        if (sym.id == INVALID_SYMBOL_ID) {
            continue;
        }

        // 5. Apply exclusion rules
        if (ShouldExclude(sym, relations, excludedMacroDecls)) {
            continue;
        }

        // 6. Check if the symbol has any reference (beyond its own definition)
        bool hasRef = HasReference(sym.id, refs);
        // For class/struct, also check if any contained constructor is referenced,
        // since constructor calls store refs under the constructor's SymbolID, not the class/struct's.
        if (!hasRef && (sym.kind == ASTKind::CLASS_DECL || sym.kind == ASTKind::STRUCT_DECL)) {
            hasRef = HasConstructorReference(sym.id, relations, refs);
        }
        if (!hasRef) {
            diagnostics.push_back(CreateUnusedDiag(
                sym.name, sym.location, GetKindDescription(sym.kind)));
        }
    }

    return diagnostics;
}

// ---------------------------------------------------------------------------
// Helper: Check if FuncParam should be skipped from unused detection.
// Skips named params, override methods, abstract/open methods, and interface methods.
// ---------------------------------------------------------------------------
static bool ShouldSkipFuncParam(const FuncParam* funcParam)
{
    if (funcParam->isNamedParam) {
        return false;
    }

    if (!funcParam->outerDecl) {
        return false;
    }

    auto outerFunc = DynamicCast<const FuncDecl*>(funcParam->outerDecl.get());
    if (!outerFunc) {
        return false;
    }

    if (outerFunc->TestAttr(Attribute::REDEF)) {
        return true;
    }

    if (outerFunc->TestAnyAttr(Attribute::ABSTRACT, Attribute::OPEN)) {
        return true;
    }

    if (outerFunc->outerDecl && outerFunc->outerDecl->astKind == ASTKind::INTERFACE_DECL) {
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Helper: Check and report unused local variable.
// ---------------------------------------------------------------------------
static void CheckUnusedVarDecl(VarDecl* varDecl, Package& package,
    const std::string& filePath, std::vector<DiagnosticToken>& diagnostics)
{
    bool isGlobalOrMember = IsGlobalOrMember(*varDecl);
    bool isLambdaVar = isLambda(*varDecl);
    if (!isGlobalOrMember && !isLambdaVar) {
        auto usages = FindDeclUsage(*varDecl, package);
        if (usages.empty()) {
            auto identifierPos = varDecl->GetIdentifierPos();
            SymbolLocation loc{
                identifierPos,
                identifierPos + CountUnicodeCharacters(varDecl->identifier),
                filePath
            };
            diagnostics.push_back(UnusedSymbolDiag::CreateUnusedDiag(
                varDecl->identifier, loc, "Variable"));
        }
    }
}

// ---------------------------------------------------------------------------
// Helper: Check and report unused function parameter.
// ---------------------------------------------------------------------------
static void CheckUnusedFuncParam(FuncParam* funcParam, Package& package,
    const std::string& filePath, std::vector<DiagnosticToken>& diagnostics)
{
    auto usages = FindDeclUsage(*funcParam, package);
    if (!usages.empty()) {
        return;
    }

    auto identifierPos = funcParam->GetIdentifierPos();
    SymbolLocation loc{
        identifierPos,
        identifierPos + CountUnicodeCharacters(funcParam->identifier),
        filePath
    };
    diagnostics.push_back(UnusedSymbolDiag::CreateUnusedDiag(
        funcParam->identifier, loc, "Parameter"));
}

// ---------------------------------------------------------------------------
// Local symbol analysis: detect unused local variables and non-named params
// ---------------------------------------------------------------------------
std::vector<DiagnosticToken> UnusedSymbolDiag::AnalyzeLocalSymbols(
    const File& file,
    Package& package)
{
    std::vector<DiagnosticToken> diagnostics;

    if (!file.curPackage) {
        return diagnostics;
    }

    std::function<VisitAction(Ptr<Node>)> collector = [&](Ptr<Node> node) -> VisitAction {
        auto decl = DynamicCast<Decl*>(node);
        if (!decl) {
            return VisitAction::WALK_CHILDREN;
        }

        if (decl->isInMacroCall || decl->curMacroCall) {
            return VisitAction::WALK_CHILDREN;
        }

        if (auto varDecl = DynamicCast<VarDecl*>(node)) {
            if (!DynamicCast<FuncParam*>(node)) {
                CheckUnusedVarDecl(varDecl, package, file.filePath, diagnostics);
            }
        }

        if (auto funcParam = DynamicCast<FuncParam*>(node)) {
            if (ShouldSkipFuncParam(funcParam)) {
                return VisitAction::WALK_CHILDREN;
            }
            CheckUnusedFuncParam(funcParam, package, file.filePath, diagnostics);
        }

        return VisitAction::WALK_CHILDREN;
    };

    Walker(const_cast<File*>(&file), collector).Walk();
    return diagnostics;
}

} // namespace ark
