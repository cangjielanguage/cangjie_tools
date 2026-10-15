// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/CommentLayout.h"
#include "cangjie/Basic/DiagnosticEngine.h"
#include "cangjie/Basic/SourceManager.h"
#include "cangjie/Lex/Lexer.h"

#include <algorithm>
#include <map>

namespace Cangjie::Format {
namespace {
struct Leaf {
    Doc* doc;
    size_t begin;
    size_t end;
};

std::string g_collectSpelling(Doc& root, std::vector<Leaf>& leaves)
{
    std::string text;
    std::vector<Doc*> pending{&root};
    while (!pending.empty()) {
        auto* doc = pending.back();
        pending.pop_back();
        switch (doc->type) {
            case DocType::LINE_DOT:
                text += "\n";
                [[fallthrough]];
            case DocType::STRING:
            case DocType::DOT:
                leaves.push_back({doc, text.size(), text.size() + doc->value.size()});
                text += doc->value;
                break;
            case DocType::LINE:
            case DocType::HARDLINE:
            case DocType::SEPARATE:
                text += "\n";
                break;
            case DocType::SOFTLINE_WITH_SPACE:
                text += " ";
                break;
            default:
                for (auto it = doc->members.rbegin(); it != doc->members.rend(); ++it) {
                    pending.emplace_back(&*it);
                }
                break;
        }
    }
    return text;
}

bool IsCode(const Token& token)
{
    return token.kind != TokenKind::NL && token.kind != TokenKind::COMMENT;
}

struct Reservation {
    std::string text;
    bool boundary;
};

bool HasComments(const std::vector<Token>& tokens)
{
    return std::any_of(tokens.begin(), tokens.end(), [](const Token& token) {
        return token.kind == TokenKind::COMMENT;
    });
}

std::vector<const Token*> GetCodeTokens(const std::vector<Token>& tokens)
{
    std::vector<const Token*> result;
    for (const auto& token : tokens) {
        if (IsCode(token)) {
            result.emplace_back(&token);
        }
    }
    return result;
}

bool HaveSameTokens(const std::vector<const Token*>& original, const std::vector<const Token*>& generated)
{
    if (original.size() != generated.size()) {
        return false;
    }
    for (size_t i = 0; i < original.size(); ++i) {
        if (original[i]->kind != generated[i]->kind || original[i]->Value() != generated[i]->Value()) {
            return false;
        }
    }
    return true;
}

std::vector<size_t> GetLineStarts(const std::string& spelling)
{
    std::vector<size_t> lineStarts{0};
    for (size_t i = 0; i < spelling.size(); ++i) {
        if (spelling[i] == '\n') {
            lineStarts.emplace_back(i + 1);
        }
    }
    return lineStarts;
}

struct ReservationContext {
    const std::vector<const Token*>& original;
    const std::vector<const Token*>& generated;
    const std::vector<size_t>& lineStarts;
    const std::vector<Leaf>& leaves;
    std::map<Doc*, std::map<size_t, std::vector<Reservation>>>& budgets;
    size_t codeIndex = 0;
    size_t leafIndex = 0;
};

bool IsCommentOnCodeLine(const Token& token, const ReservationContext& context)
{
    return context.codeIndex != 0 &&
        token.Begin().line == context.original[context.codeIndex - 1]->End().line;
}

void AddCommentReservation(const Token& token, ReservationContext& context)
{
    if (token.kind == TokenKind::NL) {
        return;
    }
    if (token.kind != TokenKind::COMMENT) {
        ++context.codeIndex;
        return;
    }
    if (!IsCommentOnCodeLine(token, context)) {
        return;
    }
    auto end = context.generated[context.codeIndex - 1]->End();
    auto offset = context.lineStarts.at(static_cast<size_t>(end.line - 1)) +
        static_cast<size_t>(end.column - 1);
    while (context.leafIndex < context.leaves.size() && context.leaves[context.leafIndex].end < offset) {
        ++context.leafIndex;
    }
    if (context.leafIndex == context.leaves.size() || offset <= context.leaves[context.leafIndex].begin) {
        return;
    }
    auto text = token.Value();
    bool boundary = text.rfind("//", 0) == 0 || text.find('\n') != std::string::npos;
    if (boundary) {
        text = text.substr(0, text.find_first_of("\r\n"));
    }
    context.budgets[context.leaves[context.leafIndex].doc][offset -
        context.leaves[context.leafIndex].begin].push_back({" " + text, boundary});
}

void AddCommentReservations(const std::vector<Token>& tokens, ReservationContext& context)
{
    for (const auto& token : tokens) {
        AddCommentReservation(token, context);
    }
}

void AppendReservationDocs(Doc& annotated, const Doc& leaf,
    const std::vector<Reservation>& reservations)
{
    for (const auto& reservation : reservations) {
        annotated.members.emplace_back(DocType::LINE_SUFFIX, leaf.indent, reservation.text);
        if (reservation.boundary) {
            annotated.members.emplace_back(DocType::LINE_SUFFIX_BOUNDARY, leaf.indent + 1, "");
        }
    }
}

void AnnotateCommentLeaf(Doc* leaf, const std::map<size_t, std::vector<Reservation>>& atOffsets)
{
    Doc annotated(DocType::CONCAT, leaf->indent, "");
    size_t previous = 0;
    for (const auto& [offset, reservations] : atOffsets) {
        Doc part = *leaf;
        part.value = leaf->value.substr(previous, offset - previous);
        annotated.members.emplace_back(std::move(part));
        AppendReservationDocs(annotated, *leaf, reservations);
        previous = offset;
    }
    if (previous < leaf->value.size()) {
        annotated.members.emplace_back(DocType::STRING, leaf->indent, leaf->value.substr(previous));
    }
    *leaf = std::move(annotated);
}

void AnnotateCommentLeaves(const std::map<Doc*, std::map<size_t, std::vector<Reservation>>>& budgets)
{
    for (const auto& [leaf, atOffsets] : budgets) {
        AnnotateCommentLeaf(leaf, atOffsets);
    }
}
}

void AddCommentBudgets(Doc& root, const std::vector<Token>& tokens)
{
    if (!HasComments(tokens)) {
        return;
    }
    std::vector<Leaf> leaves;
    auto spelling = g_collectSpelling(root, leaves);
    // Keep the synthetic source and lexer diagnostics local so a matching input
    // path cannot replace the original source buffer or affect its diagnostics.
    SourceManager sm;
    DiagnosticEngine diag;
    diag.SetSourceManager(&sm);
    auto fileID = sm.AddSource("___commentLayout.cj", spelling);
    auto generatedTokens = Lexer(fileID, spelling, diag, sm).GetTokens();
    auto original = GetCodeTokens(tokens);
    auto generated = GetCodeTokens(generatedTokens);
    // Strict alignment avoids moving a comment across reordered modifiers,
    // raw macro tokens or a partially formatted region. Recovery stays intact.
    if (!HaveSameTokens(original, generated)) {
        return;
    }
    auto lineStarts = GetLineStarts(spelling);
    std::map<Doc*, std::map<size_t, std::vector<Reservation>>> budgets;
    ReservationContext context{original, generated, lineStarts, leaves, budgets};
    AddCommentReservations(tokens, context);
    AnnotateCommentLeaves(budgets);
}
}
