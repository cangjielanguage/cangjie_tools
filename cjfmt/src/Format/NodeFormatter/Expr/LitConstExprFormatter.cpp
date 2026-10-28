// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Expr/LitConstExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

namespace {
void AppendInterpolationText(const std::string& text, std::string& result, bool& pendingSpace)
{
    if (text.empty()) {
        return;
    }
    if (pendingSpace && !result.empty() && result.back() != ' ' && text.front() != ' ') {
        result += ' ';
    }
    result += text;
    pendingSpace = false;
}

void ProcessInterpolationDoc(const Doc& doc, std::vector<const Doc*>& pending,
    std::string& result, bool& pendingSpace)
{
    switch (doc.type) {
        case DocType::LINE:
        case DocType::HARDLINE:
        case DocType::SEPARATE:
        case DocType::SOFTLINE_WITH_SPACE:
            pendingSpace = true;
            break;
        case DocType::LINE_DOT:
            pendingSpace = false;
            [[fallthrough]];
        case DocType::STRING:
        case DocType::DOT:
            AppendInterpolationText(doc.value, result, pendingSpace);
            break;
        default:
            for (auto it = doc.members.rbegin(); it != doc.members.rend(); ++it) {
                pending.emplace_back(&*it);
            }
            break;
    }
}
} // namespace

std::string LitConstExprFormatter::FormatSingleLineInterpolation(const Doc& doc)
{
    // Flatten layout Docs rather than rendered source. Rewriting whitespace in
    // the source would also change nested string literals and their values.
    std::string result;
    bool pendingSpace = false;
    std::vector<const Doc*> pending{&doc};
    while (!pending.empty()) {
        const auto* current = pending.back();
        pending.pop_back();
        ProcessInterpolationDoc(*current, pending, result, pendingSpace);
    }
    return result;
}

void LitConstExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto expr = As<ASTKind::LIT_CONST_EXPR>(node);
    AddLitConstExpr(doc, *expr, level);
}

void LitConstExprFormatter::AddLitConstExpr(Doc& doc, const Cangjie::AST::LitConstExpr& litConstExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.isStringLiteral = litConstExpr.kind == LitConstKind::STRING ||
        litConstExpr.kind == LitConstKind::JSTRING;

    std::string strValue;
    std::string quote = litConstExpr.isSingleQuote ? "'" : "\"";
    if (litConstExpr.kind == LitConstKind::STRING || litConstExpr.kind == LitConstKind::JSTRING) {
        if (litConstExpr.stringKind == StringKind::MULTILINE_RAW) {
            AddMultiLineRaw(doc, litConstExpr, quote, strValue, level);
        } else if (litConstExpr.stringKind == StringKind::MULTILINE) {
            AddMultiLine(doc, litConstExpr, quote, strValue, level);
        } else if (litConstExpr.stringKind == StringKind::JSTRING) {
            strValue += "J" + quote;
            AddSiPartExprs(doc, litConstExpr, strValue, level, true);
            strValue += quote;
        } else {
            strValue += quote;
            AddSiPartExprs(doc, litConstExpr, strValue, level, true);
            strValue += quote;
        }
    } else if (litConstExpr.kind == LitConstKind::RUNE) {
        std::string runeSymbol = "r";
        strValue = runeSymbol + quote;
        AddSiPartExprs(doc, litConstExpr, strValue, level);
        strValue += quote;
    } else if (litConstExpr.kind == LitConstKind::UNIT) {
        if (!litConstExpr.TestAttr(Attribute::COMPILER_ADD)) {
            strValue = "()";
        }
    } else {
        AddSiPartExprs(doc, litConstExpr, strValue, level);
    }
    doc.members.emplace_back(DocType::STRING, level, strValue);
    if (litConstExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

void LitConstExprFormatter::MultiLineInterpolationExprProcessor(Doc& doc, Ptr<Expr> expr, int level)
{
    auto interpolationExpr = As<ASTKind::INTERPOLATION_EXPR>(expr);
    // A multiline string's interpolation is embedded in literal text. Its
    // enclosing AST level therefore does not describe the column where `${`
    // is emitted. Align the interpolation to that output line before applying
    // the relative body indentation.
    Doc interpolation(DocType::ALIGN, level, "");
    if (interpolationExpr->block->body.size() > 1) {
        interpolation.members.emplace_back(DocType::LINE, level + 1, "");
        for (auto& node : interpolationExpr->block->body) {
            interpolation.members.emplace_back(astToFormatSource.ASTToDoc(node, level + 1));
            if (node != interpolationExpr->block->body.back()) {
                interpolation.members.emplace_back(DocType::LINE, level + 1, "");
            }
        }
        interpolation.members.emplace_back(DocType::LINE, level, "");
    } else if (interpolationExpr->block->body.size() == 1) {
        for (auto& node : interpolationExpr->block->body) {
            interpolation.members.emplace_back(astToFormatSource.ASTToDoc(node, level));
        }
    }
    doc.members.emplace_back(std::move(interpolation));
}

void LitConstExprFormatter::AddSiPartExpr(Doc& doc, Ptr<Expr> expr, std::string& strValue, int level, bool isSingleLine)
{
    if (expr->astKind == ASTKind::LIT_CONST_EXPR) {
        auto siLitConstExpr = As<ASTKind::LIT_CONST_EXPR>(expr);
        strValue += siLitConstExpr->rawString.empty() ? siLitConstExpr->stringValue : siLitConstExpr->rawString;
    } else if (expr->astKind == ASTKind::INTERPOLATION_EXPR) {
        if (isSingleLine) {
            strValue += "${";
            auto interpolationExpr = As<ASTKind::INTERPOLATION_EXPR>(expr);
            for (auto& node : interpolationExpr->block->body) {
                auto nodeDoc = astToFormatSource.ASTToDoc(node, level);
                strValue += FormatSingleLineInterpolation(nodeDoc);
            }
            strValue += "}";
        } else {
            strValue += "${";
            doc.members.emplace_back(DocType::STRING, level, strValue);
            MultiLineInterpolationExprProcessor(doc, expr, level);
            strValue = "}";
        }
    }
}

void LitConstExprFormatter::AddSiPartExprs(
    Doc& doc, const LitConstExpr& litConstExpr, std::string& strValue, int level, bool isSingleLine)
{
    if (litConstExpr.siExpr) {
        for (auto& expr : litConstExpr.siExpr->strPartExprs) {
            AddSiPartExpr(doc, expr, strValue, level, isSingleLine);
        }
    } else {
        strValue += litConstExpr.rawString.empty() ? litConstExpr.stringValue : litConstExpr.rawString;
    }
}

void LitConstExprFormatter::AddMultiLineRaw(
    Doc& doc, const LitConstExpr& litConstExpr, const std::string& quote, std::string& strValue, int level)
{
    for (unsigned i = 0; i < litConstExpr.delimiterNum; ++i) {
        strValue += "#";
    }
    strValue += quote;
    AddSiPartExprs(doc, litConstExpr, strValue, level);
    strValue += quote;
    for (unsigned i = 0; i < litConstExpr.delimiterNum; ++i) {
        strValue += "#";
    }
}

void LitConstExprFormatter::AddMultiLine(
    Doc& doc, const LitConstExpr& litConstExpr, const std::string& quote, std::string& strValue, int level)
{
    auto tripleQuotes = quote + quote + quote;
    strValue += tripleQuotes + "\n";
    AddSiPartExprs(doc, litConstExpr, strValue, level);
    strValue += tripleQuotes;
}
} // namespace Cangjie::Format
