// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/FillTypeProcessor.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <tuple>

namespace Cangjie::Format {
namespace {
// Manually chosen work budgets (2^17), not derived from performance measurements.
// They cap the counters below, not total runtime; nested width measurements are not counted.
// Exhausting Doc traversal falls back to compatibility rendering.
constexpr size_t MAX_VISITS = 131072;
// Exhausting dynamic-programming candidate transitions falls back to deterministic greedy filling.
constexpr size_t MAX_TRANSITIONS = 131072;
// Extra costs for breaks inside nested groups: strongly protect groups that fit
// on a continuation line; otherwise apply a smaller cost to permit splitting.
constexpr int PROTECTED_GROUP_PENALTY = 1000;
constexpr int SPLIT_GROUP_PENALTY = 5;
bool IsSoftLine(const Doc& doc)
{
    return doc.type == DocType::SOFTLINE || doc.type == DocType::SOFTLINE_WITH_SPACE;
}

struct LayoutItem {
    std::vector<const Doc*> content;
    int width = 0;
    const Doc* separator = nullptr; // separator preceding this item
    int penalty = 0;
};

struct LayoutCost {
    int64_t overflow = 0;
    int64_t penalty = 0;
    size_t lines = 0;

    bool operator<(const LayoutCost& other) const
    {
        return std::tie(overflow, penalty, lines) < std::tie(other.overflow, other.penalty, other.lines);
    }
};

struct FillPlanState {
    std::vector<LayoutItem> items{1};
    bool supported = true;
    bool exhausted = false;
};

struct BreakPlan {
    std::vector<bool> breaks;
    bool exhausted = false;
};

struct FillRangeState {
    int pos;
    int suffix;
    std::vector<LayoutCost> costs;
    std::vector<size_t> ends;
    size_t transitions = 0;
};

bool IsTransparent(const Doc& doc)
{
    return doc.type == DocType::CONCAT || doc.type == DocType::GROUP ||
        doc.type == DocType::FILL || doc.type == DocType::FUNC_ARG;
}

void AddFillLeaf(const Doc& doc, const FormattingOptions& options, LayoutItem& item, bool& supported)
{
    int width = DocProcessor::MeasureFlat(doc);
    // Text leaves are indivisible, including string literals wider than the
    // limit. Keep their full width in the plan so surrounding legal breaks
    // still reduce overflow without splitting or rewriting the literal.
    supported = width >= 0 && (doc.type == DocType::STRING || doc.type == DocType::DOT ||
        width + doc.indent * options.indentWidth <= options.lineLength);
    if (supported) {
        item.content.emplace_back(&doc);
        item.width += width;
    }
}

FillPlanState CollectFillItems(const Doc& root, const FormattingOptions& options)
{
    FillPlanState state;
    std::vector<std::pair<const Doc*, int>> pending{{&root, 0}};
    size_t visits = 0;
    while (!pending.empty() && state.supported && ++visits <= MAX_VISITS) {
        auto [doc, protection] = pending.back();
        pending.pop_back();
        if (IsSoftLine(*doc)) {
            LayoutItem item;
            item.separator = doc;
            item.penalty = protection + (doc->breakPenalty > 0
                ? doc->breakPenalty : DocProcessor::DefaultBreakPenalty(doc->breakKind));
            state.items.emplace_back(std::move(item));
        } else if (IsTransparent(*doc)) {
            if (doc != &root && (doc->layout == LayoutPolicy::GROUP || doc->type == DocType::FILL)) {
                int width = DocProcessor::MeasureFlat(*doc);
                protection += width >= 0 && width + (doc->indent + 1) * options.indentWidth <= options.lineLength
                    ? PROTECTED_GROUP_PENALTY : SPLIT_GROUP_PENALTY;
            }
            for (auto it = doc->members.rbegin(); it != doc->members.rend(); ++it) {
                pending.emplace_back(&*it, protection);
            }
        } else {
            AddFillLeaf(*doc, options, state.items.back(), state.supported);
        }
    }
    state.exhausted = !pending.empty();
    return state;
}

void BuildFillPlan(const std::vector<LayoutItem>& items, const std::vector<bool>& breaks,
    std::vector<Doc>& planned)
{
    planned.clear();
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) {
            const auto& separator = *items[i].separator;
            planned.emplace_back(breaks[i] ? DocType::LINE : DocType::STRING, separator.indent,
                !breaks[i] && separator.type == DocType::SOFTLINE_WITH_SPACE ? " " : "");
        }
        for (const auto* doc : items[i].content) {
            planned.emplace_back(*doc);
        }
    }
}

bool PlanFillRange(size_t start, const std::vector<LayoutItem>& items,
    const FormattingOptions& options, FillRangeState& state)
{
    const size_t count = items.size();
    int64_t column = start == 0 ? state.pos : items[start].separator->indent * options.indentWidth;
    LayoutCost best{std::numeric_limits<int64_t>::max(), 0, 0};
    for (size_t end = start; end < count; ++end) {
        if (++state.transitions > MAX_TRANSITIONS) {
            return false;
        }
        if (end > start) {
            column += items[end].separator->type == DocType::SOFTLINE_WITH_SPACE ? 1 : 0;
        }
        column += items[end].width;
        int64_t endColumn = column + (end + 1 == count ? state.suffix : 0);
        LayoutCost candidate = state.costs[end + 1];
        candidate.overflow += std::max<int64_t>(0, endColumn - options.lineLength);
        candidate.penalty += end + 1 < count ? items[end + 1].penalty : 0;
        ++candidate.lines;
        if (!(best < candidate)) {
            best = candidate;
            state.ends[start] = end + 1;
        }
        if (column > options.lineLength) {
            break;
        }
    }
    state.costs[start] = best;
    return true;
}

BreakPlan PlanFillBreaks(const std::vector<LayoutItem>& items, int pos, int suffix,
    const FormattingOptions& options)
{
    const size_t count = items.size();
    FillRangeState state{pos, suffix, std::vector<LayoutCost>(count + 1), std::vector<size_t>(count)};
    BreakPlan result;
    for (size_t i = count; i-- > 0 && !result.exhausted;) {
        result.exhausted = !PlanFillRange(i, items, options, state);
    }
    result.breaks.assign(count, false);
    if (result.exhausted) {
        int64_t column = pos;
        for (size_t i = 0; i < count; ++i) {
            int space = i > 0 && items[i].separator->type == DocType::SOFTLINE_WITH_SPACE ? 1 : 0;
            int tail = i + 1 == count ? suffix : 0;
            if (i > 0 && column + space + items[i].width + tail > options.lineLength) {
                result.breaks[i] = true;
                column = items[i].separator->indent * options.indentWidth;
                space = 0;
            }
            column += space + items[i].width;
        }
    } else {
        for (size_t i = state.ends[0]; i < count; i = state.ends[i]) {
            result.breaks[i] = true;
        }
    }
    return result;
}
} // namespace

bool FillTypeProcessor::PlanFill(const Doc& root, int pos, const FormattingOptions& options,
    const std::vector<std::pair<Doc, Mode>>& suffixCommands, std::vector<Doc>& planned)
{
    // Flatten only transparent syntax Docs. Custom printers (arguments, method
    // chains, lambda bodies) remain atomic and keep their own rendering policy.
    // Trials read the input tree; the selected plan is a separate command list.
    auto state = CollectFillItems(root, options);
    // Forced lines and custom multiline printers remain on the compatibility
    // path. The work limit is deterministic, never based on wall-clock time.
    if (!state.supported || state.exhausted || state.items.size() == 1) {
        return false;
    }
    auto& items = state.items;

    int suffix = MeasureInlineSuffix(suffixCommands);
    if (suffix < 0) {
        return false;
    }
    auto breakPlan = PlanFillBreaks(items, pos, suffix, options);
    BuildFillPlan(items, breakPlan.breaks, planned);
    return true;
}

void FillTypeProcessor::DocToString(std::string&, int& pos, std::pair<Doc, Mode>& current,
    std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    std::vector<Doc> planned;
    if (!PlanFill(current.first, pos, options, leftCmd, planned)) {
        for (auto it = current.first.members.rbegin(); it != current.first.members.rend(); ++it) {
            leftCmd.emplace_back(*it, current.second);
        }
        return;
    }
    for (auto it = planned.rbegin(); it != planned.rend(); ++it) {
        leftCmd.emplace_back(std::move(*it), Mode::MODE_FLAT);
    }
}
} // namespace Cangjie::Format
