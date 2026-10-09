// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/CommentHandler.h"
#include "Format/SimultaneousIterator.h"
#include "cangjie/Basic/DiagnosticEngine.h"

#include <algorithm>
#include <optional>

namespace Cangjie::Format {
using namespace Cangjie;
using namespace Cangjie::AST;

namespace {
const int MAX_LINEBREAKS = 2;

bool IsClosing(const Token &token)
{
    return token.kind == TokenKind::RCURL || token.kind == TokenKind::RPAREN || token.kind == TokenKind::RSQUARE;
}

bool IsBlockContinuationKeyword(const Token &token)
{
    return token.kind == TokenKind::ELSE || token.kind == TokenKind::CATCH ||
        token.kind == TokenKind::FINALLY || token.kind == TokenKind::WHILE;
}

void EnsureSpaceAfterStar(std::string &line, size_t starPosition)
{
    auto firstNonStarSymbolPosition = line.find_first_not_of('*', starPosition);
    if (firstNonStarSymbolPosition == 0 || firstNonStarSymbolPosition == std::string::npos ||
        line.size() <= firstNonStarSymbolPosition || std::isspace(line[firstNonStarSymbolPosition]) ||
        line[firstNonStarSymbolPosition] == '/') {
        return;
    }
    (void)line.insert(firstNonStarSymbolPosition, " ");
}

void FormatLineFromMultilineComment(std::string &line, int lineNumber, int linesCount, int indent,
    std::string &output)
{
    if (lineNumber == 0) {
        output.append(static_cast<size_t>(std::max(indent, 0)), ' ');
        EnsureSpaceAfterStar(line, 1);
        return;
    }
    int originalIndentation = static_cast<int>(line.find_first_not_of(' '));
    Utils::TrimString(line);
    if (line.empty()) {
        return;
    }
    auto startsWithStar = line.front() == '*';
    auto isSimpleLastLine = lineNumber == linesCount - 1 && line == "*/";
    int minIndentationFromFirstLine = isSimpleLastLine ? 1 : (startsWithStar ? 1 : 0);
    auto indentation = startsWithStar ? indent + minIndentationFromFirstLine :
        std::max(originalIndentation, indent + minIndentationFromFirstLine);
    output.append(static_cast<size_t>(std::max(indentation, 0)), ' ');
    EnsureSpaceAfterStar(line, 0);
}

int GetCommentEndLine(const Token &token)
{
    // Token::End() does not reliably account for newlines inside block comments.
    const auto &value = token.Value();
    return token.Begin().line + static_cast<int>(std::count(value.begin(), value.end(), '\n'));
}

struct InsertData {
    std::optional<Token> tokenBefore;
    std::optional<Token> tokenAfter;
    std::vector<Token> comments;

    InsertData(const std::optional<Token> tokenBefore, const std::optional<Token> tokenAfter,
        const std::vector<Token> comments)
        : tokenBefore(tokenBefore), tokenAfter(tokenAfter), comments(comments)
    {}
};

class CommentHandler : private SimultaneousIterator {
public:
    // Formatting options are loaded per invocation, so the configured width must not be cached in a global value.
    CommentHandler(const std::vector<Token> &originalTokens, const std::vector<Token> &outputTokens, SourceManager &sm,
        int indentWidth, std::optional<std::pair<int, int>> selectedLines)
        : SimultaneousIterator(originalTokens, outputTokens), sm(sm), indentWidth(indentWidth),
          selectedLines(selectedLines)
    {
        this->outputText = "";
    }

    std::string DoInsert()
    {
        if (originalTokens.empty()) {
            return outputText;
        }
        std::vector<Token> startOfFileComments = IterateAndCollectComments();
        if (!startOfFileComments.empty()) {
            auto firstNonCommentToken =
                originalIterator != originalTokens.end() ? std::optional(*originalIterator) : std::nullopt;
            InsertComments(InsertData(std::nullopt, firstNonCommentToken, startOfFileComments));
        }

        if (outputIterator == outputTokens.end()) {
            return outputText;
        }
        Position prevPosition = outputIterator->Begin();
        Position currentPosition = outputIterator->End();
        while (originalIterator != originalTokens.end() || outputIterator != outputTokens.end()) {
            auto commentsToInsertAfter = AdvanceAndComputeInsertData();
            if (!commentsToInsertAfter) {
                if (outputIterator != outputTokens.end()) {
                    currentPosition = outputIterator->End();
                } else if (prevPosition < currentPosition) {
                    InsertTextBetween(prevPosition, currentPosition);
                    prevPosition = currentPosition;
                }
                continue;
            }

            CheckCurrentPos(currentPosition);

            InsertTextBetween(prevPosition, currentPosition);
            InsertComments(*commentsToInsertAfter);

            if (outputIterator == outputTokens.end()) {
                continue;
            }

            prevPosition = outputIterator->Begin();
            currentPosition = outputIterator->End();

            if (std::prev(outputIterator)->kind == TokenKind::NL && outputText.back() == '\n') {
                Insert(' ', outputIterator->Begin().column - 1);
            } else if (std::prev(outputIterator)->kind != TokenKind::NL && outputText.back() == '\n') {
                Insert(' ', ComputeIndentFor(outputIterator, false));
            }
        }
        return outputText;
    }

private:
    SourceManager &sm;
    int indentWidth;
    std::optional<std::pair<int, int>> selectedLines;
    std::string outputText;

    void Insert(char character, int times)
    {
        for (int i = 0; i < times; ++i) {
            outputText.push_back(character);
        }
    }

    void InsertTextBetween(const Position &begin, const Position &end)
    {
        (void)outputText.append(sm.GetContentBetween(begin, end));
    }

    void CheckCurrentPos(Position& currentPosition)
    {
        if (std::prev(outputIterator)->kind == TokenKind::SEMI) {
            currentPosition = std::prev(outputIterator)->End();
        }

        if (std::prev(outputIterator)->kind == TokenKind::NL) {
            if (std::prev(std::prev(outputIterator))->kind == TokenKind::SEMI) {
                currentPosition = std::prev(std::prev(outputIterator))->End();
            }
        }

        if (std::prev(outputIterator)->kind == TokenKind::NL) {
            if (std::prev(std::prev(outputIterator))->kind == TokenKind::NL) {
                if (std::prev(std::prev(std::prev(outputIterator)))->kind == TokenKind::SEMI) {
                    currentPosition = std::prev(std::prev(std::prev(outputIterator)))->End();
                }
            }
        }
    }

    std::vector<Token>::const_iterator FindFirstTokenOnThisLine(std::vector<Token>::const_iterator token)
    {
        if (token == outputTokens.begin()) {
            return token;
        }
        for (;;) {
            auto prev = std::prev(token);
            if (prev == outputTokens.begin()) {
                return prev;
            }
            if (prev->kind == TokenKind::NL) {
                return token;
            }
            --token;
        }
    }

    /*
     * This find correct indentation when the formatting was split by eol comment.
     * When isComment is true, we're effectively trying to find for the token before this one.
     * The basic idea is when we're indenting the start of the line then the indent
     * is always the same as the formatted code.
     * If we need to split the line then the code would be indented one level.
     * Everything else is trying to add custom behaviour for common special cases so the formatting looks more natural.
     */
    int ComputeIndentFor(const std::vector<Token>::const_iterator tokenToIndent, bool isComment)
    {
        if (tokenToIndent == outputTokens.end()) {
            return 0;
        }
        auto firstOnThisLine = FindFirstTokenOnThisLine(tokenToIndent);
        auto sameIndent = (firstOnThisLine->Begin().column - 1);
        auto oneLevelIndented = sameIndent + indentWidth;

        if (!isComment && tokenToIndent == firstOnThisLine) {
            return sameIndent;
        }

        auto isFirstOnThisLine = tokenToIndent == firstOnThisLine;
        auto isLastOnThisLine = std::next(tokenToIndent) == outputTokens.end() ||
            std::next(tokenToIndent)->kind == TokenKind::NL;

        auto isLastClosingBrace = isLastOnThisLine && IsClosing(*tokenToIndent);
        if (isComment && isLastClosingBrace) {
            return oneLevelIndented;
        }
        if (!isComment && isLastClosingBrace) {
            return sameIndent;
        }
        if (!isComment && tokenToIndent->kind == TokenKind::LCURL) {
            return sameIndent;
        }
        if (!isFirstOnThisLine) {
            // A comment can split "} else/catch/finally/while" across lines.
            // Keep the continuation keyword aligned with the preceding block.
            if (!isComment && std::prev(tokenToIndent)->kind == TokenKind::RCURL &&
                IsBlockContinuationKeyword(*tokenToIndent)) {
                return sameIndent;
            }
            // pattern matching 'case' always creats indented region
            if (firstOnThisLine->kind == TokenKind::CASE) {
                return oneLevelIndented;
            }
            // arrow in lambda or 'case' clauses can be formatted in various ways
            if (firstOnThisLine->kind == TokenKind::DOUBLE_ARROW ||
                std::prev(tokenToIndent)->kind == TokenKind::DOUBLE_ARROW ||
                tokenToIndent->kind == TokenKind::DOUBLE_ARROW) {
                return sameIndent;
            }
            return oneLevelIndented;
        }
        return sameIndent;
    }

    std::optional<InsertData> AdvanceAndComputeInsertData()
    {
        std::optional<Token> tokenBefore = std::nullopt;

        if (originalIterator != originalTokens.end()) {
            tokenBefore = std::optional(*originalIterator);
        }

        auto commentsBetween = Advance();
        if (commentsBetween.empty()) {
            return std::nullopt;
        }
        std::optional<Token> tokenAfter =
            (originalIterator != originalTokens.end()) ? std::optional(*originalIterator) : std::nullopt;

        return InsertData(tokenBefore, tokenAfter, commentsBetween);
    }

    void InsertComment(const Token &token, int indent)
    {
        auto commentEndLine = GetCommentEndLine(token);
        auto format = !selectedLines.has_value() ||
            (token.Begin().line <= selectedLines->second && commentEndLine >= selectedLines->first);
        if (format) {
            (void)outputText.append(FormatCommentToken(token, indent));
        } else {
            Insert(' ', indent);
            (void)outputText.append(token.Value());
        }
    }

    bool IsDoubleSeparate()
    {
        if (outputIterator != outputTokens.begin() && std::prev(outputIterator)->kind == TokenKind::NL &&
            std::prev(outputIterator) != outputTokens.begin() &&
            prev(std::prev(outputIterator))->kind == TokenKind::NL) {
            return true;
        }
        return false;
    }

    void InsertComments(const InsertData &insertData)
    {
        auto &comments = insertData.comments;
        auto it = comments.begin();

        auto nextNonCommentTokenPosition =
            insertData.tokenAfter ? insertData.tokenAfter->Begin() : Position(comments.back().Begin().line + 1, 1);

        bool isReadyInsertNL = false;
        while (it != comments.end()) {
            auto token = *it;

            // manage whitespace characters before each of the comments
            Position prevTokenPos;
            if (it == comments.begin()) {
                prevTokenPos = insertData.tokenBefore ? insertData.tokenBefore->End() : Position(1, 1);
            } else {
                prevTokenPos = std::prev(it)->End();
                prevTokenPos.line = GetCommentEndLine(*std::prev(it));
            }

            auto lineBreaksBefore = token.Begin().line - prevTokenPos.line;
            Insert('\n', std::min(!isReadyInsertNL ? MAX_LINEBREAKS : MAX_LINEBREAKS - 1, lineBreaksBefore));
            if (lineBreaksBefore == 0 && insertData.tokenBefore) {
                Insert(' ', 1);
            }

            auto shouldAlignToNextNonCommentToken = lineBreaksBefore > 0;
            if (shouldAlignToNextNonCommentToken) {
                InsertComment(token, ComputeIndentFor(outputIterator, true));
            } else {
                InsertComment(token, 0);
            }

            if (lineBreaksBefore == 0 && IsDoubleSeparate()) {
                Insert('\n', 1);
                isReadyInsertNL = true;
            }

            ++it;
        }

        // manage whitespace characters after the last comment

        auto lastCommentEndLine =
            insertData.tokenAfter ? GetCommentEndLine(comments.back()) : comments.back().End().line;
        auto lineBreaksAfter = nextNonCommentTokenPosition.line - lastCommentEndLine;

        Insert('\n', std::min(!isReadyInsertNL ? MAX_LINEBREAKS : MAX_LINEBREAKS - 1, lineBreaksAfter));
        if (lineBreaksAfter == 0) {
            Insert(' ', 1);
        }
    }
};
}

std::string FormatCommentToken(const Token &token, int indent)
{
    if (token.Value().rfind("/*", 0) != 0 || token.Value().find('\n') == std::string::npos) {
        return std::string(static_cast<size_t>(std::max(indent, 0)), ' ') + token.Value();
    }
    auto lines = Utils::SplitLines(token.Value());
    std::string output;
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string line = lines[i];
        FormatLineFromMultilineComment(line, static_cast<int>(i), static_cast<int>(lines.size()), indent, output);
        output.append(line);
        if (i + 1 != lines.size()) {
            output.push_back('\n');
        }
    }
    return output;
}

std::string InsertComments(const std::vector<Token> &originalTokens, const std::vector<Token> &formattedTokens,
    SourceManager &sm, const FormattingOptions &options, std::optional<std::pair<int, int>> selectedLines)
{
    auto commentHandler = CommentHandler(originalTokens, formattedTokens, sm, options.indentWidth, selectedLines);
    return commentHandler.DoInsert();
}
} // namespace Cangjie::Format
