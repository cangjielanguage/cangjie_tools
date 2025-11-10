// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_SIMULTANEOUSITERATOR_H
#define CJFMT_SIMULTANEOUSITERATOR_H

#include "cangjie/Lex/Token.h"
#include "cangjie/Basic/SourceManager.h"

#include <string>
#include <vector>
#include <optional>
#include <utility>

namespace Cangjie::Format {

// iterates over two tokenstreams simulteneously
// originalTokens correspond to text before formatting
// outputTokens correspond to text after formatting
// both token streams should correspond to the same 'fragment' of text
// ignores new line tokens
// allows to iterate over matching pairs of tokens
// can collect comments from original tokens stream
class SimultaneousIterator {
public:
    SimultaneousIterator(const std::vector<Token> &originalTokens, const std::vector<Token> &outputTokens);
    // originalIterator skips comments and nl and is set at next non-comment token
    // returns a vector of skipped comment tokens
    std::vector<Token> IterateAndCollectComments();
    // advances both iterators (skipping comments and nl) and sets both to next matching pair of tokens,
    // returns a vector of skipped comment tokens
    std::vector<Token> Advance();

    const std::vector<Token> &originalTokens;
    const std::vector<Token> &outputTokens;
    std::vector<Token>::const_iterator originalIterator;
    std::vector<Token>::const_iterator outputIterator;

private:
    std::optional<std::vector<Token>> RecoverOnMismatch();
};

// find subset of original tokens between [startPosition, endPosition] and corresponding subset of output tokens
std::optional<std::pair<std::vector<Token>, std::vector<Token>>> ExtractTokensBetweenPositions(
    const std::vector<Token> &originalTokens, const std::vector<Token> &formattedTokens,
    const Cangjie::Position &startPosition, const Cangjie::Position &endPosition
);

} // namespace Cangjie::Format

#endif // CJFMT_SIMULTANEOUSITERATOR_H