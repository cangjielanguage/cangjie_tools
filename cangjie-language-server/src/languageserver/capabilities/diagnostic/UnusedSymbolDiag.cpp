// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "UnusedSymbolDiag.h"
#include "../../CompilerCangjieProject.h"
#include "../../common/FindDeclUsage.h"
#include "../../common/Utils.h"
#include "cangjie/AST/Node.h"
#include "cangjie/AST/Walker.h"
#include <chrono>

namespace ark {
using namespace Cangjie;
using namespace AST;
using namespace lsp;

// ---------------------------------------------------------------------------
// Constants for macro-generated symbol patterns
// ---------------------------------------------------------------------------
namespace {
    // Macro names
    constexpr const char* COMPONENT_MACRO = "Component";
    constexpr const char* ENTRY_MACRO = "Entry";

    // Generated variable prefixes
    constexpr const char* STATE_VAR_DECL_PREFIX = "stateVarDecl_";
    constexpr const char* A_P_PREFIX = "A_P_";

    // Macro-generated function name patterns
    constexpr const char* GET_PREFIX = "get_";
    constexpr const char* SET_PREFIX = "set_";
    constexpr const char* UNDERSCORE_PREFIX = "__";
    constexpr const char* ABOUT_TO_PREFIX = "aboutTo";
    constexpr const char* PURGE_PREFIX = "purge";
    constexpr const char* UPDATE_WITH_VALUE_PARAMS = "updateWithValueParams";
    constexpr const char* INIT_FUNC = "init";
    constexpr const char* ABOUT_TO_APPEAR = "aboutToAppear";
    constexpr const char* ABOUT_TO_DISAPPEAR = "aboutToDisappear";
}

namespace {
bool MatchesRawUnusedDiagnostic(Ptr<const Decl> decl, const DiagnosticToken& diagnostic, const ArkAST& ast)
{
    if (!decl || !decl->identifier.IsRaw() || decl->curMacroCall || decl->isInMacroCall ||
        decl->identifier.Begin().fileID != ast.fileID) {
        return false;
    }
    auto pos = decl->GetIdentifierPos();
    if (decl->astKind != ASTKind::FUNC_PARAM && !decl->TestAttr(Attribute::ENUM_CONSTRUCTOR) &&
        (IsGlobalOrMember(*decl) || decl->astKind == ASTKind::FUNC_DECL ||
        (isLambda(*decl) && diagnostic.message.find("Lambda '") == 0))) {
        PositionUTF8ToIDE(ast.tokens, pos, *decl);
    }
    const auto name = std::string(decl->identifier);
    return pos.line == diagnostic.range.start.line + 1 && pos.column == diagnostic.range.start.column + 1 &&
        diagnostic.message.find("'" + name + "'") != std::string::npos;
}

std::optional<int> GetRawUnusedDiagnosticLength(const DiagnosticToken& diagnostic, const ArkAST& ast)
{
    if (!diagnostic.diagFix || !diagnostic.diagFix->removeUnusedSymbol || !ast.sourceManager || !ast.file) {
        return std::nullopt;
    }
    std::optional<int> length;
    ConstWalker(ast.file, [&](Ptr<const Node> node) {
        auto decl = DynamicCast<const Decl*>(node);
        if (!MatchesRawUnusedDiagnostic(decl, diagnostic, ast)) {
            return VisitAction::WALK_CHILDREN;
        }
        const auto begin = decl->identifier.Begin();
        const auto end = decl->identifier.End();
        if (begin.line <= 0 || begin.column <= 0 || end.fileID != begin.fileID || end.line != begin.line ||
            end.column <= begin.column) {
            return VisitAction::WALK_CHILDREN;
        }
        const auto sourceName = ast.sourceManager->GetContentBetween(begin, end);
        if (sourceName.empty() || sourceName.size() != static_cast<size_t>(end.column - begin.column)) {
            return VisitAction::WALK_CHILDREN;
        }
        length = CountUnicodeCharacters(sourceName);
        return VisitAction::STOP_NOW;
    }).Walk();
    return length;
}

void UpdateRawUnusedDiagnosticRanges(std::vector<DiagnosticToken>& diagnostics, const std::string& filePath)
{
    if (diagnostics.empty()) {
        return;
    }
    auto* project = CompilerCangjieProject::GetInstance();
    auto* ast = project ? project->GetArkAST(filePath) : nullptr;
    if (!ast || !std::any_of(ast->tokens.begin(), ast->tokens.end(), [](const Token& token) {
        return token.Value().find('`') != std::string::npos;
    })) {
        return;
    }
    for (auto& diagnostic : diagnostics) {
        if (auto length = GetRawUnusedDiagnosticLength(diagnostic, *ast)) {
            // Use the source spelling for the length without changing the diagnostic start.
            diagnostic.range.end.column = diagnostic.range.start.column + *length;
        }
    }
}
} // namespace

// ---------------------------------------------------------------------------
// Helper: Check if a function name matches macro-generated patterns
// ---------------------------------------------------------------------------
static bool IsMacroGeneratedFuncName(const std::string& funcName)
{
    static const std::unordered_set<std::string> exactMatches = {
        INIT_FUNC,
        ABOUT_TO_APPEAR,
        ABOUT_TO_DISAPPEAR,
        UPDATE_WITH_VALUE_PARAMS
    };

    return exactMatches.count(funcName) > 0 ||
           funcName.find(GET_PREFIX) == 0 ||
           funcName.find(SET_PREFIX) == 0 ||
           funcName.find(UNDERSCORE_PREFIX) == 0 ||
           funcName.find(ABOUT_TO_PREFIX) == 0 ||
           funcName.find(PURGE_PREFIX) == 0 ||
           funcName.find('.') != std::string::npos;
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
    if ((symbol.modifier == lsp::Modifier::PUBLIC || symbol.modifier == lsp::Modifier::PROTECTED) &&
        symbol.kind != ASTKind::TYPE_ALIAS_DECL) {
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
    const SymbolIndex& index,
    const std::unordered_set<SymbolID>& excludedMacroDecls)
{
    if (ShouldExcludeByTypeOrModifier(symbol)) {
        return true;
    }

    if (symbol.kind == ASTKind::FUNC_DECL || symbol.kind == ASTKind::PROP_DECL) {
        if (index.IsSymbolOverridden(symbol.id)) {
            return true;
        }
    }

    if ((symbol.kind == ASTKind::CLASS_DECL || symbol.kind == ASTKind::STRUCT_DECL) &&
        excludedMacroDecls.count(symbol.id) > 0) {
        return true;
    }

    // Skip @Component/@Entry generated internal symbols
    auto symName = symbol.name;

    // Skip variables with specific prefixes
    if (symName.find(STATE_VAR_DECL_PREFIX) == 0 || symName.find(A_P_PREFIX) == 0) {
        return true;
    }

    // Skip macro-generated functions
    if (symbol.kind == ASTKind::FUNC_DECL && IsMacroGeneratedFuncName(symName)) {
        return true;
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
        case ASTKind::LAMBDA_EXPR:
            return "Lambda";
        case ASTKind::CLASS_DECL:
            return "Class";
        case ASTKind::STRUCT_DECL:
            return "Struct";
        case ASTKind::ENUM_DECL:
            return "Enum";
        case ASTKind::INTERFACE_DECL:
            return "Interface";
        case ASTKind::TYPE_ALIAS_DECL:
            return "Type alias";
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
    diagToken.diagFix = DiagFix{false, false, true};  // Enable removeUnusedSymbol QuickFix
    return diagToken;
}

// ---------------------------------------------------------------------------
// Main analysis: detect unused global/member symbols from the index
// ---------------------------------------------------------------------------
std::vector<DiagnosticToken> UnusedSymbolDiag::Analyze(
    const SymbolIndex& index,
    const std::string& pkgName,
    const std::string& filePath,
    const Package* package)
{
    std::vector<DiagnosticToken> diagnostics;

    auto excludedMacroDecls = CollectExcludedMacroDecls(package);

    index.ForEachFileSymbol(pkgName, filePath, [&](const Symbol& sym) -> bool {
        if (UNUSED_CHECK_KINDS.count(sym.kind) == 0) {
            return true;
        }
        if (sym.location.IsZeroLoc()) {
            return true;
        }
        if (sym.id == INVALID_SYMBOL_ID) {
            return true;
        }
        if (ShouldExclude(sym, index, excludedMacroDecls)) {
            return true;
        }

        bool hasRef = index.HasSymbolReference(sym.id);
        if (!hasRef && (sym.kind == ASTKind::CLASS_DECL || sym.kind == ASTKind::STRUCT_DECL)) {
            hasRef = index.HasReferencedConstructorChild(sym.id);
        }
        if (!hasRef) {
            diagnostics.push_back(CreateUnusedDiag(
                sym.name, sym.location, GetKindDescription(sym.kind)));
        }
        return true;
    });

    UpdateRawUnusedDiagnosticRanges(diagnostics, filePath);
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

    if (outerFunc->outerDecl && outerFunc->outerDecl->astKind == ASTKind::ENUM_DECL) {
        return true;
    }

    if (outerFunc->outerDecl && outerFunc->outerDecl->astKind == ASTKind::INTERFACE_DECL) {
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Helper: Check if a node is in @Component/@Entry macro-expanded code
// ---------------------------------------------------------------------------
static bool IsInComponentMacro(const Node* node)
{
    if (!node || !node->curMacroCall || node->isInMacroCall) {
        return false;
    }

    // Check if the macro is @Component or @Entry
    auto macroCall = node->curMacroCall.get();
    if (!macroCall) {
        return false;
    }

    auto invocation = macroCall->GetConstInvocation();
    if (!invocation || !invocation->target) {
        return false;
    }

    auto macroName = invocation->target->identifier;
    return macroName == COMPONENT_MACRO || macroName == ENTRY_MACRO;
}

// ---------------------------------------------------------------------------
// Pre-built usage sets for O(1) lookup during unused-symbol detection.
// Replaces per-decl FindDeclUsage walks (each scanning the whole package)
// with a single package walk that populates these sets up front.
// ---------------------------------------------------------------------------
namespace {
struct FuncParamSourceHash {
    size_t operator()(const FuncParam* param) const noexcept
    {
        return param->begin.Hash64();
    }
};

struct FuncParamSourceEqual {
    bool operator()(const FuncParam* lhs, const FuncParam* rhs) const noexcept
    {
        return lhs == rhs ||
            (lhs->begin == rhs->begin && lhs->end == rhs->end && lhs->curFile == rhs->curFile);
    }
};

struct UsageSets {
    // Decls referenced directly via node->GetTarget() (pointer match).
    std::unordered_set<const Decl*> usedByPtr;
    // Default-argument lowering can clone parameters. Match only parameter
    // targets by their original declaration range and file, not call labels.
    std::unordered_set<const FuncParam*, FuncParamSourceHash, FuncParamSourceEqual> usedDefaultParamSources;
    // Decls referenced from a source position different from the decl's
    // own position. Used by enum-variant detection to filter out
    // self-position matches (macro artifacts where a decl references itself).
    std::unordered_set<const Decl*> usedByPtrExternal;

    void RecordDefaultParamSource(const Decl& target)
    {
        if (target.astKind != ASTKind::FUNC_PARAM) {
            return;
        }
        auto param = DynamicCast<const FuncParam*>(&target);
        if (!param) {
            return;
        }
        auto outerFunc = DynamicCast<const FuncDecl*>(param->outerDecl);
        if (outerFunc && outerFunc->ownerFunc) {
            (void)usedDefaultParamSources.insert(param);
        }
    }

    bool IsUsed(const Decl& decl) const
    {
        if (usedByPtr.count(&decl) > 0) {
            return true;
        }
        if (decl.astKind != ASTKind::FUNC_PARAM) {
            return false;
        }
        auto param = DynamicCast<const FuncParam*>(&decl);
        return param && usedDefaultParamSources.count(param) > 0;
    }

    bool IsUsedExternal(const Decl& decl) const
    {
        return usedByPtrExternal.count(&decl) > 0;
    }
};
}  // namespace

// ---------------------------------------------------------------------------
// Helper: Check and report unused local variable.
// ---------------------------------------------------------------------------
static void CheckUnusedVarDecl(VarDecl* varDecl,
    const std::string& filePath, std::vector<DiagnosticToken>& diagnostics,
    const UsageSets& usage)
{
    // Skip variables in @Component/@Entry macro-expanded code
    if (IsInComponentMacro(varDecl)) {
        return;
    }

    // Skip @Component/@Entry generated internal variables
    std::string varName(varDecl->identifier);
    if (varName.find(STATE_VAR_DECL_PREFIX) == 0) {
        return;
    }

    bool isGlobalOrMember = IsGlobalOrMember(*varDecl);
    if (!isGlobalOrMember) {
        if (!usage.IsUsed(*varDecl)) {
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
// Helper: Check if a parameter is auto-generated by @Component/@Entry macros.
// These parameters (elmtId, isInitialRender) are added by the macro system
// and users cannot control them, so they should not be reported as unused.
// ---------------------------------------------------------------------------
static bool IsComponentGeneratedParam(const FuncParam* funcParam)
{
    if (!funcParam || !funcParam->curMacroCall || funcParam->isInMacroCall) {
        return false;
    }

    // Check if this is a known @Component/@Entry generated parameter name
    static const std::unordered_set<std::string> componentParams = {
        "elmtId",
        "isInitialRender"
    };

    return componentParams.count(std::string(funcParam->identifier)) > 0;
}

// ---------------------------------------------------------------------------
// Helper: Check if a FuncParam belongs to a macro-generated method.
// Macro-generated methods: get_*, set_*, __*, aboutTo*, purge*, init, updateWithValueParams
// Also includes functions with '.' in name (e.g., bb.1, cc.2) generated by @Component/@Entry
// ---------------------------------------------------------------------------
static bool IsInMacroGeneratedFunc(const FuncParam* funcParam)
{
    if (!funcParam || !funcParam->outerDecl) {
        return false;
    }
    auto outerFunc = DynamicCast<const FuncDecl*>(funcParam->outerDecl.get());
    if (!outerFunc) {
        return false;
    }

    return IsMacroGeneratedFuncName(outerFunc->identifier);
}

// ---------------------------------------------------------------------------
// Helper: Check and report unused function parameter.
// ---------------------------------------------------------------------------
static void CheckUnusedFuncParam(FuncParam* funcParam,
    const std::string& filePath, std::vector<DiagnosticToken>& diagnostics,
    const UsageSets& usage)
{
    auto outerFunc = DynamicCast<const FuncDecl*>(funcParam->outerDecl);
    if (outerFunc && outerFunc->ownerFunc) {
        // Default-value helper parameters are copies of source parameters.
        // Diagnose only the source declaration, while still walking the body.
        return;
    }
    bool isComponentGen = IsComponentGeneratedParam(funcParam);
    bool isInCompMacro = IsInComponentMacro(funcParam);
    bool isInMacroGenFunc = IsInMacroGeneratedFunc(funcParam);

    // Skip @Component/@Entry auto-generated parameters
    if (isComponentGen) {
        return;
    }

    // Skip parameters in @Component/@Entry macro-expanded code,
    // BUT only if they are in macro-generated methods.
    // User-defined methods (like name(dd:String)) should still be checked.
    if (isInCompMacro && isInMacroGenFunc) {
        return;
    }

    if (usage.IsUsed(*funcParam)) {
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
// Helper: Check and report unused enum variant (enum constructor).
// Enum variants can be FuncDecl (with associated values) or VarDecl
// (without associated values), both stored in EnumDecl::constructors.
// ---------------------------------------------------------------------------
static void CheckUnusedEnumVariantCommon(Decl& decl,
    const std::string& filePath, std::vector<DiagnosticToken>& diagnostics,
    const SymbolIndex* index, const UsageSets& usage)
{
    if (index != nullptr) {
        auto symId = GetDeclSymbolID(decl);
        if (symId != INVALID_SYMBOL_ID && index->HasSymbolReference(symId)) {
            return;
        }
    }
    if (usage.IsUsedExternal(decl)) {
        return;
    }
    auto identifierPos = decl.GetIdentifierPos();
    SymbolLocation loc{
        identifierPos,
        identifierPos + CountUnicodeCharacters(decl.identifier),
        filePath
    };
    diagnostics.push_back(UnusedSymbolDiag::CreateUnusedDiag(
        decl.identifier, loc, "Enum variant"));
}

// ---------------------------------------------------------------------------
// Walker visitor for local symbol analysis
// ---------------------------------------------------------------------------
static void HandleMacroCallFuncParam(FuncParam* funcParam,
    const std::string& filePath, std::vector<DiagnosticToken>& diagnostics,
    const UsageSets& usage)
{
    if (!funcParam || funcParam->isMemberParam) {
        return;
    }
    if (!IsInMacroGeneratedFunc(funcParam) && !ShouldSkipFuncParam(funcParam)) {
        CheckUnusedFuncParam(funcParam, filePath, diagnostics, usage);
    }
}

static void HandleVarDecl(Ptr<Node> node,
    const std::string& filePath, std::vector<DiagnosticToken>& diagnostics,
    const UsageSets& usage)
{
    auto varDecl = DynamicCast<VarDecl*>(node);
    if (varDecl && !DynamicCast<FuncParam*>(node)) {
        CheckUnusedVarDecl(varDecl, filePath, diagnostics, usage);
    }
}

static void HandleFuncParam(FuncParam* funcParam,
    const std::string& filePath, std::vector<DiagnosticToken>& diagnostics,
    const UsageSets& usage)
{
    // Skip member params (e.g. `public let a: String` in primary constructors).
    // These are class members handled by the index layer (Analyze() checks
    // ShouldExclude -> symbol.isMemberParam), not local symbols.
    // HandleMacroCallFuncParam already checks isMemberParam; this mirrors
    // that check for the non-macro-call path.
    if (!funcParam) {
        return;
    }
    if (!ShouldSkipFuncParam(funcParam)) {
        CheckUnusedFuncParam(funcParam, filePath, diagnostics, usage);
    }
}

static void HandleEnumDecl(Ptr<Node> node,
    const std::string& filePath, std::vector<DiagnosticToken>& diagnostics,
    const SymbolIndex* index, const UsageSets& usage)
{
    auto enumDecl = DynamicCast<EnumDecl*>(node);
    if (!enumDecl) {
        return;
    }
    for (auto& ctor : enumDecl->constructors) {
        if (ctor && !ctor->isInMacroCall) {
            CheckUnusedEnumVariantCommon(*ctor, filePath, diagnostics, index, usage);
        }
    }
}

static VisitAction CollectUnusedLocalDiags(
    Ptr<Node> node,
    const std::string& filePath,
    std::vector<DiagnosticToken>& diagnostics,
    const SymbolIndex* index,
    const UsageSets& usage)
{
    auto decl = DynamicCast<Decl*>(node);
    if (!decl || decl->TestAttr(Attribute::IMPLICIT_ADD)) {
        return VisitAction::WALK_CHILDREN;
    }

    if (decl->astKind == ASTKind::PRIMARY_CTOR_DECL) {
        return VisitAction::SKIP_CHILDREN;
    }

    auto funcParam = DynamicCast<FuncParam*>(node);

    if (decl->isInMacroCall) {
        HandleMacroCallFuncParam(funcParam, filePath, diagnostics, usage);
        return VisitAction::WALK_CHILDREN;
    }

    HandleVarDecl(node, filePath, diagnostics, usage);
    HandleFuncParam(funcParam, filePath, diagnostics, usage);
    HandleEnumDecl(node, filePath, diagnostics, index, usage);

    return VisitAction::WALK_CHILDREN;
}

// Collect declaration usage in one package walk for the diagnostic hash lookups.
// Track direct references, source parameter reads in default-value helpers,
// and references outside declaration positions used to filter enum self-references.
static void BuildUsageSets(Package& package, UsageSets& usage)
{
    std::function<VisitAction(Ptr<Node>)> collector =
        [&usage, &collector](Ptr<Node> node) -> VisitAction {
        // File nodes: recurse into macro-expanded and trashBin subtrees,
        // matching FindUsage's behavior so macro/desugar refs are captured.
        if (auto fileNode = DynamicCast<File*>(node)) {
            for (auto& it : fileNode->originalMacroCallNodes) {
                Walker(it.get(), collector).Walk();
            }
            for (auto& it : fileNode->trashBin) {
                Walker(it.get(), collector).Walk();
            }
        }
        // Only default-value helper targets need source-identity matching.
        if (auto target = node->GetTarget()) {
            if (usage.usedByPtr.insert(target).second) {
                usage.RecordDefaultParamSource(*target);
            }
            if (node->begin != target->begin || node->end != target->end) {
                (void)usage.usedByPtrExternal.insert(target);
            }
        } else if (DynamicCast<NameReferenceExpr*>(node.get()) != nullptr) {
            // DEBUG: log RefExpr nodes with null target
            auto filePath = node->curFile ? node->curFile->filePath : "null";
            Trace::Log("NameRefExpr null target:", "file =", filePath,
                "line =", node->begin.line, "col =", node->begin.column);
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker(&package, collector).Walk();
}

// ---------------------------------------------------------------------------
// Local symbol analysis: detect unused local variables and non-named params
// ---------------------------------------------------------------------------
std::vector<DiagnosticToken> UnusedSymbolDiag::AnalyzeLocalSymbols(
    const File& file,
    Package& package,
    const SymbolIndex* index)
{
    std::vector<DiagnosticToken> diagnostics;

    if (!file.curPackage) {
        return diagnostics;
    }

    // Phase 1: single package-wide walk to build usage sets.
    // Replaces O(D) nested FindDeclUsage walks (each scanning the whole
    // package) with one O(P x F) walk followed by O(D) hash lookups.
    UsageSets usage;
    BuildUsageSets(package, usage);

    // Phase 2: walk the current file only, dispatching to Check* helpers
    // which now consult the pre-built usage sets instead of triggering
    // nested full-package walks.
    std::function<VisitAction(Ptr<Node>)> collector =
        [&](Ptr<Node> node) -> VisitAction {
            return CollectUnusedLocalDiags(node, file.filePath,
                                           diagnostics, index, usage);
        };

    Walker(const_cast<File*>(&file), collector).Walk();
    UpdateRawUnusedDiagnosticRanges(diagnostics, file.filePath);
    return diagnostics;
}

} // namespace ark
