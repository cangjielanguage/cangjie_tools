// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/SimultaneousIterator.h"

namespace Cangjie::Format {

namespace {
bool IsComment(const Token &tok)
{
    return tok.kind == TokenKind::COMMENT;
}

bool IsCommentOrNL(const Token &tok)
{
    return tok.kind == TokenKind::COMMENT || tok.kind == TokenKind::NL;
}

bool IsModifier(const Token &tok)
{
    return tok.kind >= TokenKind::STATIC && tok.kind <= TokenKind::OPERATOR;
}

bool AdvanceToStartPosition(SimultaneousIterator &simIter, const Cangjie::Position &startPosition)
{
    while (simIter.originalIterator != simIter.originalTokens.end() &&
        simIter.outputIterator != simIter.outputTokens.end()) {
        if (simIter.originalIterator->Begin() > startPosition) {
            return false;
        }
        if (simIter.originalIterator->Begin() == startPosition) {
            return true;
        }
        if (simIter.originalIterator->End() == startPosition) {
            (void)simIter.Advance();
            return true;
        }
        (void)simIter.Advance();
    }
    return false;
}
}

SimultaneousIterator::SimultaneousIterator(
    const std::vector<Token> &originalTokens, const std::vector<Token> &outputTokens)
    : originalTokens(originalTokens), outputTokens(outputTokens)
{
    this->originalIterator = this->originalTokens.begin();
    this->outputIterator = this->outputTokens.begin();
}

std::vector<Token> SimultaneousIterator::IterateAndCollectComments()
{
    std::vector<Token> comments;
    while (originalIterator != originalTokens.end() && IsCommentOrNL(*originalIterator)) {
        if (IsComment(*originalIterator)) {
            comments.push_back(*originalIterator);
        }
        ++originalIterator;
    }
    return comments;
}

std::vector<Token> SimultaneousIterator::Advance()
{
    std::vector<Token> commentsBetween;
    if (originalIterator != originalTokens.end()) {
        ++originalIterator;
        while (originalIterator != originalTokens.end() && originalIterator->kind == TokenKind::SEMI) {
            ++originalIterator;
        }
        commentsBetween = IterateAndCollectComments();
    }

    if (outputIterator != outputTokens.end()) {
        ++outputIterator;

        while (outputIterator != outputTokens.end() &&
            (IsCommentOrNL(*outputIterator) || outputIterator->kind == TokenKind::SEMI)) {
            ++outputIterator;
        }
    }
    auto recovered = RecoverOnMismatch();
    if (recovered.has_value()) {
        auto commentsSkippedWhileRecovering = recovered.value();
        (void)commentsBetween.insert(commentsBetween.end(),
            commentsSkippedWhileRecovering.begin(), commentsSkippedWhileRecovering.end());
    }

    return commentsBetween;
}

std::optional<std::vector<Token>> SimultaneousIterator::RecoverOnMismatch()
{
    if (originalIterator == originalTokens.end() || outputIterator == outputTokens.end()) {
        return std::nullopt;
    }
    if (originalIterator->Value() == outputIterator->Value()) {
        return std::nullopt;
    }

    if (IsModifier(*originalIterator) && IsModifier(*outputIterator)) {
        // formatter can reorder modifiers
        return std::nullopt;
    }

    // try to recover to next non-whitespace token
    // first try to move original iterator forward
    // because we could delete tokens such as semicolon
    // so the output iterator would be ahead of original
    std::vector<Token> skippedComments;
    auto originalRecoverCandidate = std::next(originalIterator);
    while (originalRecoverCandidate != originalTokens.end() && IsCommentOrNL(*originalRecoverCandidate)) {
        if (IsComment(*originalRecoverCandidate)) {
            skippedComments.push_back(*originalRecoverCandidate);
        }
        ++originalRecoverCandidate;
    }
    if (originalRecoverCandidate != originalTokens.end() &&
        originalRecoverCandidate->Value() == outputIterator->Value()) {
        originalIterator = originalRecoverCandidate;
        return skippedComments;
    }

    auto outputRecoverCandidate = std::next(outputIterator);
    while (outputRecoverCandidate != outputTokens.end() && IsCommentOrNL(*outputRecoverCandidate)) {
        ++outputRecoverCandidate;
    }
    if (outputRecoverCandidate != outputTokens.end() &&
        outputRecoverCandidate->Value() == originalIterator->Value()) {
        outputIterator = outputRecoverCandidate;
        return std::nullopt;
    }

    return std::nullopt;
}

std::optional<std::pair<std::vector<Token>, std::vector<Token>>> ExtractTokensBetweenPositions(
    const std::vector<Token> &originalTokens, const std::vector<Token> &formattedTokens,
    const Cangjie::Position &startPosition, const Cangjie::Position &endPosition
    )
{
    auto simIter = SimultaneousIterator(originalTokens, formattedTokens);
    (void)simIter.IterateAndCollectComments(); // skip comments at the begining of original text
    if (!AdvanceToStartPosition(simIter, startPosition)) {
        return std::nullopt;
    }
    auto originalTokensSubsetFirst = simIter.originalIterator;
    auto outputTokensSubsetFirst = simIter.outputIterator;
    std::vector<Token>::const_iterator originalTokensSubsetEnd;
    std::vector<Token>::const_iterator outputTokensSubsetEnd;
    while (true) {
        if (simIter.originalIterator == simIter.originalTokens.end() ||
         simIter.outputIterator == simIter.outputTokens.end() || simIter.originalIterator->Begin() > endPosition) {
            return std::nullopt;
        }
        if (simIter.originalIterator->Begin() == endPosition) {
            originalTokensSubsetEnd = simIter.originalIterator;
            outputTokensSubsetEnd = simIter.outputIterator;
            break;
        }
        if (simIter.originalIterator->End() == endPosition) {
            originalTokensSubsetEnd = std::next(simIter.originalIterator);
            outputTokensSubsetEnd = std::next(simIter.outputIterator);
            break;
        }
        (void)simIter.Advance();
    }
    auto origFragment = std::vector<Token>(originalTokensSubsetFirst, originalTokensSubsetEnd);
    auto outputFragment = std::vector<Token>(outputTokensSubsetFirst, outputTokensSubsetEnd);
    if (origFragment.empty() || outputFragment.empty()) {
        return std::nullopt;
    }
    while (origFragment.back().kind == TokenKind::NL) {
        origFragment.pop_back();
    }
    while (outputFragment.back().kind == TokenKind::NL) {
        outputFragment.pop_back();
    }
    if (origFragment.empty() || outputFragment.empty()) {
        return std::nullopt;
    }
    return std::make_pair(std::move(origFragment), std::move(outputFragment));
}

} // namespace Cangjie::Format
