// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/LitConstExprFormatter.h"
#include <regex>
#include <string>
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

std::string LitConstExprFormatter::CleanString(const std::string& input)
{
    std::string result = std::regex_replace(input, std::regex(R"()" + options.newLine + R"()"), "");
    result = std::regex_replace(result, std::regex(R"(\s+\.)"), ".");
    result = std::regex_replace(result, std::regex(R"(\s+)"), " ");
    result = std::regex_replace(result, std::regex(R"(^\s+|\s+$)"), "");

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
    if (interpolationExpr->block->body.size() > 1) {
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        for (auto& node : interpolationExpr->block->body) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(node, level + 1));
            if (node != interpolationExpr->block->body.back()) {
                doc.members.emplace_back(DocType::LINE, level + 1, "");
            }
        }
        doc.members.emplace_back(DocType::LINE, level, "");
    } else if (interpolationExpr->block->body.size() == 1) {
        for (auto& node : interpolationExpr->block->body) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(node, level));
        }
    }
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
                strValue += CleanString(astToFormatSource.DocToString(nodeDoc));
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
