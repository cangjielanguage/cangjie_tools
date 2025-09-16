// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_ASTTOFORMATSOURCE_H
#define CJFMT_ASTTOFORMATSOURCE_H

#include "Format/Doc.h"
#include "Format/DocProcessor/DocProcessor.h"
#include "Format/NodeFormatter/NodeFormatter.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/AST/Match.h"
#include "cangjie/AST/Node.h"
#include "cangjie/Basic/Utils.h"
#include "cangjie/Parse/Parser.h"

#include <limits>
#include <optional>

namespace Cangjie::Format {
static const Cangjie::Position INVALID_POSITION = Cangjie::Position{0, 0, 0};

struct Region {
    int startLine;
    int endLine;
    bool isWholeFile;

    Region() = default;
    Region(int start, int end, bool isWholeFile) noexcept : startLine(start), endLine(end), isWholeFile(isWholeFile) {}

    static const Region wholeFile;
};

struct RegionFormattingTracker {
    Cangjie::Position shouldFormatBegin;
    Cangjie::Position shouldFormatEnd;
    std::optional<Cangjie::Position> actuallyFormattedStart; // defines bounds of the top level ast nodes
    std::optional<Cangjie::Position> actuallyFormattedEnd;   // touched during fragment formatting
    std::optional<Cangjie::Position> cuttingPointOutsideRegionBegin;
    std::optional<Cangjie::Position> cuttingPointInsideRegionBegin;
    std::optional<Cangjie::Position> cuttingPointOutsideRegionEnd;
    std::optional<Cangjie::Position> cuttingPointInsideRegionEnd;

    RegionFormattingTracker() = default;

    explicit RegionFormattingTracker(const Region regionToFormat)
        : shouldFormatBegin(regionToFormat.startLine, 1),
          shouldFormatEnd(regionToFormat.endLine, std::numeric_limits<int>::max()),
          actuallyFormattedStart(std::nullopt),
          actuallyFormattedEnd(std::nullopt),
          cuttingPointOutsideRegionBegin(std::nullopt),
          cuttingPointInsideRegionBegin(std::nullopt),
          cuttingPointOutsideRegionEnd(std::nullopt),
          cuttingPointInsideRegionEnd(std::nullopt)
    {
    }

    void ProcessNodeFormatted(Ptr<Cangjie::AST::Node> node);
    bool IsInsideFormattedNode(Ptr<Cangjie::AST::Node> node) const;
    bool ShouldFormat(Ptr<Cangjie::AST::Node> node);
    std::optional<std::pair<Cangjie::Position, Cangjie::Position>> GetPreciseFragmentBounds(const SourceManager& sm);

private:
    void ProcessPotentialCuttingPoint(Cangjie::Position& pos);
};

class ASTToFormatSource {
public:
    RegionFormattingTracker& tracker;
    Cangjie::SourceManager& sm;

    ASTToFormatSource(RegionFormattingTracker& tracker, Cangjie::SourceManager& sm) : tracker(tracker), sm(sm) {}
    Doc ASTToDoc(Ptr<Cangjie::AST::Node> node, int level = 0, FuncOptions funcOptions = FuncOptions());
    std::string DocToString(Doc& doc);
    std::string DocToString(Doc& doc, int& pos, std::string& formatted);
    void AddAnnotations(Doc& doc, const std::vector<OwnedPtr<Cangjie::AST::Annotation>>& annotations, int level,
        bool changeLine = true);
    void AddGenericParams(Doc& doc, const Cangjie::AST::Generic& generic, int level);
    void AddGenericBound(Doc& doc, const Cangjie::AST::Generic& generic, int level);
    void AddBreakLineParam(
        Doc& doc, const Cangjie::AST::FuncParamList& funcParamList, int level, FuncOptions funcOptions);
    void AddMatchSelector(Doc& doc, const Cangjie::AST::MatchExpr& matchExpr, int level);
    void EditMacroStr(const Token& attr, std::string& macroStr, TokenKind& preTokenKind);
    bool WithoutSpace(TokenKind preTokenKind) const;
    bool IsMultipleLineArg(const std::vector<OwnedPtr<Cangjie::AST::FuncArg>>& args);
    bool IsMultipleLineCallExpr(const Cangjie::AST::CallExpr& callExpr) const;
    bool IsMultipleLineArrayLit(const int& rightSquarePosLine,
        const std::vector<OwnedPtr<Cangjie::AST::Expr>>& children) const;
    bool IsMultipleLineExpr(const std::vector<OwnedPtr<Cangjie::AST::Expr>>& children);
    bool IsMultipleLine(const OwnedPtr<Cangjie::AST::Expr>& expr);
    int DepthInMultipleMethodChain(const Cangjie::AST::MemberAccess& memberAccess);

    template <typename T> void MacroProcessor(const T& macro, Doc& doc, int level)
    {
        std::string macroStr;

        TokenKind preTokenKind = TokenKind::ILLEGAL;

        for (auto& attr : macro->invocation.attrs) {
            EditMacroStr(attr, macroStr, preTokenKind);
        }

        doc.members.emplace_back(DocType::STRING, level, macroStr);
    }

    template <typename T>
    void AnnotationProcessor(const T& macro, Doc& doc, int level, bool& isMultipleLine, AST::Annotation& annotation)
    {
        isMultipleLine = macro->invocation.rightSquarePos.line != macro->invocation.attrs.back().End().line;
        FuncOptions funcOptions;
        funcOptions.isInsideBuildNode = true;
        if (isMultipleLine) {
            doc.members.emplace_back(DocType::LINE, level + 1, "");
            for (auto& arg : annotation.args) {
                doc.members.emplace_back(ASTToDoc(arg.get(), level + 1, funcOptions));
                if (arg->commaPos != INVALID_POSITION) {
                    doc.members.emplace_back(DocType::STRING, level + 1, ",");
                }
                if (arg != annotation.args.back()) {
                    doc.members.emplace_back(DocType::LINE, level + 1, "");
                }
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        } else {
            for (auto& arg : annotation.args) {
                doc.members.emplace_back(ASTToDoc(arg.get(), level, funcOptions));
                if (arg->commaPos != INVALID_POSITION) {
                    doc.members.emplace_back(DocType::STRING, level, ",");
                }
                if (arg != annotation.args.back()) {
                    doc.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
                }
            }
        }
    }

    template <typename T> void AddMacroExpandNode(Doc& doc, const T& macro, int level, bool patternOrEnum,
        bool isParam, FuncOptions& funcOptions)
    {
        doc.type = DocType::CONCAT;
        doc.indent = level;
        if (!macro->annotations.empty()) {
            AddAnnotations(doc, macro->annotations, level);
        }

        std::string compileTimeVisibleStr = macro->invocation.isCompileTimeVisible ? "!" : "";
        doc.members.emplace_back(DocType::STRING, level, "@" + compileTimeVisibleStr + macro->invocation.fullName);
        auto argStr = sm.GetContentBetween(macro->invocation.leftSquarePos.fileID, macro->invocation.leftSquarePos,
            macro->invocation.rightSquarePos + 1);
        DiagnosticEngine diag;
        Parser parser(argStr, diag, sm);
        auto annotation = MakeOwned<AST::Annotation>();
        parser.ParseAnnotationArguments(*annotation);

        bool hasInvalidExpr = false;
        if (parser.GetDiagnosticEngine().GetErrorCount() > 0) {
            hasInvalidExpr = true;
        }

        if (macro->invocation.leftSquarePos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level, "[");
        }

        if (hasInvalidExpr) {
            MacroProcessor(macro, doc, level);
        }

        bool isMultipleLine = false;
        if (!annotation->args.empty() && !hasInvalidExpr) {
            AnnotationProcessor(macro, doc, level, isMultipleLine, *annotation);
        }
        if (macro->invocation.rightSquarePos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level, "]");
        }

        if (macro->invocation.leftParenPos != INVALID_POSITION && macro->invocation.rightParenPos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level,
                sm.GetContentBetween(macro->invocation.leftParenPos.fileID, macro->invocation.leftParenPos,
                    macro->invocation.rightParenPos + 1));
        }

        if (macro->invocation.decl != nullptr) {
            if (!funcOptions.isMultipleLineMacroExpendParam &&
                macro->invocation.decl->astKind == AST::ASTKind::MACRO_EXPAND_PARAM) {
                funcOptions.isMultipleLineMacroExpendParam = true;
            }
            if (isParam && !isMultipleLine && !funcOptions.isMultipleLineMacroExpendParam) {
                doc.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level, "");
            } else {
                doc.members.emplace_back(DocType::LINE, level, "");
            }
        }
        funcOptions.patternOrEnum = patternOrEnum;
        doc.members.emplace_back(ASTToDoc(macro->invocation.decl.get(), level, funcOptions));
    };

    template <typename T> void AddEmptyBody(Doc& doc, const T&, int level, bool isSameLine = true)
    {
        doc.members.emplace_back(DocType::STRING, level, " {");
        if (!isSameLine) {
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(DocType::STRING, level, "}");
    };

    template <typename T> void AddBodyMembers(Doc& doc, const std::vector<T>& members, int level)
    {
        int lastEndLine = -1;
        for (auto& n : members) {
            if (lastEndLine != -1) {
                if (n->begin.line > lastEndLine + 1) {
                    doc.members.emplace_back(DocType::SEPARATE, level, "");
                }
                doc.members.emplace_back(DocType::LINE, level, "");
            }
            doc.members.emplace_back(ASTToDoc(n.get(), level));
            lastEndLine = n->end.line;
        }
    };

    void AddModifier(Doc& doc, const std::set<Cangjie::AST::Modifier>& modifiers, int level);
    void AddModifier(Doc& doc, Cangjie::AST::Modifier& modifier, int level);

    template <typename T, typename... Args> void RegisterNode(AST::ASTKind kind, Args&&... args)
    {
        formatNodeMap[kind] = std::make_shared<T>(*this, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args> void RegisterDocProcessors(DocType type, Args&&... args)
    {
        toStringMap[type] = std::make_shared<T>(*this, std::forward<Args>(args)...);
    }

private:
    std::map<AST::ASTKind, std::shared_ptr<NodeFormatter>> formatNodeMap;
    std::map<DocType, std::shared_ptr<DocProcessor>> toStringMap;
};
} // namespace Cangjie::Format

#endif // CJFMT_ASTTOFORMATSOURCE_H
